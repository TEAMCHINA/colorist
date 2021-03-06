// ---------------------------------------------------------------------------
//                         Copyright Joe Drago 2018.
//         Distributed under the Boost Software License, Version 1.0.
//            (See accompanying file LICENSE_1_0.txt or copy at
//                  http://www.boost.org/LICENSE_1_0.txt)
// ---------------------------------------------------------------------------

#ifndef COLORIST_TRANSFORM_H
#define COLORIST_TRANSFORM_H

#include "colorist/types.h"

// for gbMat3
#include "gb_math.h"

// for cmsHPROFILE, cmsHTRANSFORM
#include "lcms2.h"

struct clContext;
struct clProfile;

// Why the X's in these enums? Transform, XForm, get it? (I needed to disambiguate)

typedef enum clTransformFormat
{
    CL_XF_XYZ_FLOAT = 0,
    CL_XF_RGB_FLOAT,
    CL_XF_RGBA_FLOAT,
    CL_XF_RGB_8,
    CL_XF_RGBA_8,
    CL_XF_RGB_16,
    CL_XF_RGBA_16
} clTransformFormat;

typedef enum clTransformTransferFunction
{
    CL_XTF_NONE = 0,
    CL_XTF_GAMMA,
    CL_XTF_PQ // partial support
} clTransformTransferFunction;

// clTransform does not own either clProfile and it is expected that both will outlive the clTransform that uses them
typedef struct clTransform
{
    struct clProfile * srcProfile; // If NULL, is XYZ profile
    struct clProfile * dstProfile; // If NULL, is XYZ profile
    clTransformFormat srcFormat;
    clTransformFormat dstFormat;

    // Cache for CCMM objects
    clTransformTransferFunction srcEOTF;
    clTransformTransferFunction dstOETF;
    float srcGamma;
    float dstInvGamma;
    gbMat3 matSrcToDst;
    clBool ccmmReady;

    // Cache for LittleCMS objects
    cmsHPROFILE xyzProfile;
    cmsHTRANSFORM hTransform;
} clTransform;

clTransform * clTransformCreate(struct clContext * C, struct clProfile * srcProfile, clTransformFormat srcFormat, struct clProfile * dstProfile, clTransformFormat dstFormat);
void clTransformDestroy(struct clContext * C, clTransform * transform);
clBool clTransformUsesCCMM(struct clContext * C, clTransform * transform);
const char * clTransformCMMName(struct clContext * C, clTransform * transform); // Convenience function
void clTransformRun(struct clContext * C, clTransform * transform, int taskCount, void * srcPixels, void * dstPixels, int pixelCount);

clBool clTransformFormatIsFloat(struct clContext * C, clTransformFormat format);
int clTransformFormatToPixelBytes(struct clContext * C, clTransformFormat format);

// if X+Y+Z is 0, clTransformXYZToXYY() returns (whitePointX, whitePointY, 0)
void clTransformXYZToXYY(struct clContext * C, float * dstXYY, float * srcXYZ, float whitePointX, float whitePointY);
void clTransformXYYToXYZ(struct clContext * C, float * dstXYZ, float * srcXYY);

#endif // ifndef COLORIST_TRANSFORM_H
