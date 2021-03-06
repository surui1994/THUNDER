/*******************************************************************************
 * Author: Mingxu Hu, Hongkun Yu
 * Dependency:
 * Test:
 * Execution:
 * Description:
 *
 * Manual:
 * ****************************************************************************/

#ifndef FFT_H
#define FFT_H

#ifdef SINGLE_PRECISION
#include <fftw3.h>
#else
#include <fftw3.h>
#endif

#include "Logging.h"
#include "Precision.h"

#include <omp_compat.h>

#include "Config.h"
#include "Complex.h"

#include "Image.h"
#include "Volume.h"
#include "ImageFunctions.h"

/**
 * This macro checks whether the source and destination of Fourier transform are
 * both repaired.
 *
 * @param dst the destination of Fourier transform
 * @param src the source of Fourier transform
 */
#define CHECK_SPACE_VALID(dst, src) \
{ \
    if (src == NULL) \
    { \
        REPORT_ERROR("FFT Needs Input Data."); \
        abort(); \
    } \
    if (dst == NULL) \
    { \
        REPORT_ERROR("FFT Needs Output Data."); \
        abort(); \
    } \
}

#define FW_EXTRACT_P(obj) \
{ \
    obj.alloc(FT_SPACE); \
    _dstC = (TSFFTW_COMPLEX*)&obj[0]; \
    _srcR = &obj(0); \
    CHECK_SPACE_VALID(_dstC, _srcR); \
}

/**
 * This macro destroys the plan for performing Fourier transform and assigns
 * the pointers to NULL.
 */
#define FW_CLEAN_UP \
{ \
    _Pragma("omp critical (line67)"); \
    TSFFTW_destroy_plan(fwPlan); \
    fwPlan = NULL; \
    _dstC = NULL; \
    _srcR = NULL; \
}

#define FW_CLEAN_UP_MT \
{ \
    TSFFTW_destroy_plan(fwPlan); \
    fwPlan = NULL; \
    _dstC = NULL; \
    _srcR = NULL; \
}

#define BW_EXTRACT_P(obj) \
{ \
    obj.alloc(RL_SPACE); \
    _dstR = &obj(0); \
    _srcC = (TSFFTW_COMPLEX*)&obj[0]; \
    CHECK_SPACE_VALID(_dstR, _srcC); \
}

/**
 * This macro destroys the plan for performing inverse Fourier transform,
 * assigns the pointers to NULL and clear up the Fourier space of the image
 * (volume).
 *
 * @param obj the image (volume) performed inverse Fourier transform.
 */
#define BW_CLEAN_UP(obj) \
{ \
    _Pragma("omp critical (line110)"); \
    TSFFTW_destroy_plan(bwPlan); \
    bwPlan = NULL; \
    _dstR = NULL; \
    _srcC = NULL; \
    obj.clearFT(); \
}

#define BW_CLEAN_UP_MT(obj) \
{ \
    TSFFTW_destroy_plan(bwPlan); \
    bwPlan = NULL; \
    _dstR = NULL; \
    _dstR = NULL; \
    _srcC = NULL; \
    obj.clearFT(); \
}

/**
 * This macro executes a function in real space and performs Fourier transform
 * on the destination image (volume).
 *
 * @param dst the destination image (volume)
 * @param src the source image (volume)
 * @param function the function to be executed
 */
#define R2C_RL(dst, src, function) \
    do \
    { \
        function; \
        FFT fft; \
        fft.fw(dst); \
    } while (0)

/**
 * This macro performs inverse Fourier transform on the source image (volume)
 * and executes a function in real space.
 *
 * @param dst the destination image (volume)
 * @param src the source image (volume)
 * @param function the function to be executed
 */
#define C2R_RL(dst, src, function) \
    do \
    { \
        FFT fft; \
        fft.bw(src); \
        function; \
    } while (0)

/**
 * This macro performs inverse Fourier transform on the source image (volume),
 * executes a function in real space and performs Fourier transform on the
 * destination image (volume).
 *
 * @param dst the destination image (volume)
 * @param src the source image (volume)
 * @param function the function to be executed
 */
#define C2C_RL(dst, src, function) \
    do \
    { \
        FFT fft; \
        fft.bw(src); \
        function; \
        fft.fw(dst); \
    } while (0)

/**
 * This macro performs Fourier transform on the source image (volume), executes
 * a function in Fourier space and perform inverse Fourier transform on the
 * destination image (volume).
 *
 * @param dst the destination image (volume)
 * @param src the source image (volume)
 * @param function the function to be executed
 */
#define R2R_FT(dst, src, function) \
    do \
    { \
        FFT fft; \
        fft.fw(src); \
        function; \
        fft.bw(dst); \
    } while (0)

/**
 * This macro performs Fourier transform on the source image (volume) and
 * excutes a function in Fourier space.
 *
 * @param dst the destination image (volume)
 * @param src the source image (volume)
 * @param function the function to be executed
 */
#define R2C_FT(dst, src, function) \
    do \
    { \
        FFT fft; \
        fft.fw(src); \
        function; \
    } while (0)

/**
 * This macro executes a function in Fourier space and performs an inverse
 * Fourier transform on the destination image (volume).
 *
 * @param dst the destination image (volume)
 * @param src the source image (volume)
 * @param function the function to be executed
 */
#define C2R_FT(dst, src, function) \
    do \
    { \
        function; \
        FFT fft; \
        fft.bw(dst); \
    } while (0)

class FFT
{
    private:

        /**
         * a pointer points to the source of the Fourier transform
         */
        RFLOAT* _srcR;

        /**
         * a pointer points to the source of the inverse Fourier transform
         */
        TSFFTW_COMPLEX* _srcC;

        /**
         * a pointer points to the destination of the inverse Fourier transform
         */
        RFLOAT* _dstR;

        /**
         * a pointer points to the destination of the Fourier transform
         */
        TSFFTW_COMPLEX* _dstC;

        /**
         * the plan of Fourier transform
         */
        TSFFTW_PLAN fwPlan;

        /**
         * the plan of inverse Fourier transform
         */
        TSFFTW_PLAN bwPlan;

    public:

        /**
         * default contructor
         */
        FFT();

        /**
         * default reconstructor
         */
        ~FFT();

        /**
         * This function performs Fourier transform on an image.
         *
         * @param img the image to be transformed
         */
        void fw(Image& img);

        /**
         * This function performs inverse Fourier transform on an image.
         *
         * @param img the image to be transformed
         */
        void bw(Image& img);

        /**
         * This function performs Fourier transform on a volume.
         *
         * @param vol the volume to be transformed
         */
        void fw(Volume& vol);

        /**
         * This function performs inverse Fourier transform on a volume.
         *
         * @param vol the volume to be transformed
         */
        void bw(Volume& vol);

        /**
         * This function performs Fourier transform on an image using multiple
         * threads.
         *
         * @param img the image to be transformed
         */
        void fwMT(Image& img);

        /**
         * This function performs inverse Fourier transform on an image using 
         * multiple threads.
         *
         * @param img the image to be transformed
         */
        void bwMT(Image& img);

        /**
         * This function performs Fourier transform on a volume using multiple
         * threads.
         *
         * @param vol the volume to be transformed
         */
        void fwMT(Volume& vol);

        /**
         * This function performs inverse Fourier transform on a volume using
         * multiple threads.
         *
         * @param vol the volume to be transformed
         */
        void bwMT(Volume& vol);

        void fwCreatePlan(const int nCol,
                          const int nRow);

        void fwCreatePlan(const int nCol,
                          const int nRow,
                          const int nSlc);

        void bwCreatePlan(const int nCol,
                          const int nRow);

        void bwCreatePlan(const int nCol,
                          const int nRow,
                          const int nSlc);

        void fwCreatePlanMT(const int nCol,
                            const int nRow);

        void fwCreatePlanMT(const int nCol,
                            const int nRow,
                            const int nSlc);

        void bwCreatePlanMT(const int nCol,
                            const int nRow);

        void bwCreatePlanMT(const int nCol,
                            const int nRow,
                            const int nSlc);

        void fwExecutePlan(Image& img);

        void fwExecutePlan(Volume& vol);

        void bwExecutePlan(Image& img);

        void bwExecutePlan(Volume& vol);

        void fwExecutePlanMT(Image& img);

        void fwExecutePlanMT(Volume& vol);

        void bwExecutePlanMT(Image& img);

        void bwExecutePlanMT(Volume& vol);

        void fwDestroyPlan();

        void bwDestroyPlan();

        void fwDestroyPlanMT();

        void bwDestroyPlanMT();
};

#endif // FFT_H 
