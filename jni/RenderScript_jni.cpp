/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#include <utils/misc.h>
#include <utils/Log.h>

#include <ui/EGLNativeWindowSurface.h>
#include <ui/Surface.h>

#include <core/SkBitmap.h>

#include "jni.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"

#include "../RenderScript.h"
#include "../RenderScriptEnv.h"

//#define LOG_API LOGE
#define LOG_API(...)

using namespace android;

// ---------------------------------------------------------------------------

static void doThrow(JNIEnv* env, const char* exc, const char* msg = NULL)
{
    jclass npeClazz = env->FindClass(exc);
    env->ThrowNew(npeClazz, msg);
}

static jfieldID gContextId = 0;
static jfieldID gNativeBitmapID = 0;

static void _nInit(JNIEnv *_env, jclass _this)
{
    gContextId             = _env->GetFieldID(_this, "mContext", "I");

    jclass bitmapClass = _env->FindClass("android/graphics/Bitmap");
    gNativeBitmapID = _env->GetFieldID(bitmapClass, "mNativeBitmap", "I");
}


// ---------------------------------------------------------------------------

static void
nAssignName(JNIEnv *_env, jobject _this, jint obj, jbyteArray str)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nAssignName, con(%p), obj(%p)", con, obj);

    jint len = _env->GetArrayLength(str);
    jbyte * cptr = (jbyte *) _env->GetPrimitiveArrayCritical(str, 0);
    rsAssignName((void *)obj, (const char *)cptr);
    _env->ReleasePrimitiveArrayCritical(str, cptr, JNI_ABORT);
}


// ---------------------------------------------------------------------------

static jint
nDeviceCreate(JNIEnv *_env, jobject _this)
{
    LOG_API("nDeviceCreate");
    return (jint)rsDeviceCreate();
}

static void
nDeviceDestroy(JNIEnv *_env, jobject _this, jint dev)
{
    LOG_API("nDeviceDestroy");
    return rsDeviceDestroy((RsDevice)dev);
}

static jint
nContextCreate(JNIEnv *_env, jobject _this, jint dev, jobject wnd, jint ver)
{
    LOG_API("nContextCreate");

    if (wnd == NULL) {
        not_valid_surface:
        doThrow(_env, "java/lang/IllegalArgumentException",
                "Make sure the SurfaceView or associated SurfaceHolder has a valid Surface");
        return 0;
    }
    jclass surface_class = _env->FindClass("android/view/Surface");
    jfieldID surfaceFieldID = _env->GetFieldID(surface_class, "mSurface", "I");
    Surface * window = (Surface*)_env->GetIntField(wnd, surfaceFieldID);
    if (window == NULL)
        goto not_valid_surface;

    LOGE("nContextCreate 5");
    return (jint)rsContextCreate((RsDevice)dev, window, ver);
}

static void
nContextDestroy(JNIEnv *_env, jobject _this, jint con)
{
    LOG_API("nContextDestroy, con(%p)", (RsContext)con);
    return rsContextDestroy((RsContext)con);
}


static void
nElementBegin(JNIEnv *_env, jobject _this)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nElementBegin, con(%p)", con);
    rsElementBegin();
}

static void
nElementAddPredefined(JNIEnv *_env, jobject _this, jint predef)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nElementAddPredefined, con(%p), predef(%i)", con, predef);
    rsElementAddPredefined((RsElementPredefined)predef);
}

static void
nElementAdd(JNIEnv *_env, jobject _this, jint kind, jint type, jint norm, jint bits)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nElementAdd, con(%p), kind(%i), type(%i), norm(%i), bits(%i)", con, kind, type, norm, bits);
    rsElementAdd((RsDataKind)kind, (RsDataType)type, norm != 0, (size_t)bits);
}

static jint
nElementCreate(JNIEnv *_env, jobject _this)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nElementCreate, con(%p)", con);
    return (jint)rsElementCreate();
}

static jint
nElementGetPredefined(JNIEnv *_env, jobject _this, jint predef)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nElementGetPredefined, con(%p) predef(%i)", con, predef);
    return (jint)rsElementGetPredefined((RsElementPredefined)predef);
}

static void
nElementDestroy(JNIEnv *_env, jobject _this, jint e)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nElementDestroy, con(%p) e(%p)", con, (RsElement)e);
    rsElementDestroy((RsElement)e);
}

// -----------------------------------

static void
nTypeBegin(JNIEnv *_env, jobject _this, jint eID)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nTypeBegin, con(%p) e(%p)", con, (RsElement)eID);
    rsTypeBegin((RsElement)eID);
}

static void
nTypeAdd(JNIEnv *_env, jobject _this, jint dim, jint val)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nTypeAdd, con(%p) dim(%i), val(%i)", con, dim, val);
    rsTypeAdd((RsDimension)dim, val);
}

static jint
nTypeCreate(JNIEnv *_env, jobject _this)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nTypeCreate, con(%p)", con);
    return (jint)rsTypeCreate();
}

static void
nTypeDestroy(JNIEnv *_env, jobject _this, jint eID)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nTypeDestroy, con(%p), t(%p)", con, (RsType)eID);
    rsTypeDestroy((RsType)eID);
}

// -----------------------------------

static jint
nAllocationCreateTyped(JNIEnv *_env, jobject _this, jint e)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nAllocationCreateTyped, con(%p), e(%p)", con, (RsElement)e);
    return (jint) rsAllocationCreateTyped((RsElement)e);
}

static jint
nAllocationCreatePredefSized(JNIEnv *_env, jobject _this, jint predef, jint count)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nAllocationCreatePredefSized, con(%p), predef(%i), count(%i)", con, predef, count);
    return (jint) rsAllocationCreatePredefSized((RsElementPredefined)predef, count);
}

static jint
nAllocationCreateSized(JNIEnv *_env, jobject _this, jint e, jint count)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nAllocationCreateSized, con(%p), e(%p), count(%i)", con, (RsElement)e, count);
    return (jint) rsAllocationCreateSized((RsElement)e, count);
}

static void
nAllocationUploadToTexture(JNIEnv *_env, jobject _this, jint a, jint mip)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nAllocationUploadToTexture, con(%p), a(%p), mip(%i)", con, (RsAllocation)a, mip);
    rsAllocationUploadToTexture((RsAllocation)a, mip);
}

static RsElementPredefined SkBitmapToPredefined(SkBitmap::Config cfg)
{
    switch (cfg) {
    case SkBitmap::kA8_Config:
        return RS_ELEMENT_A_8;
    case SkBitmap::kARGB_4444_Config:
        return RS_ELEMENT_RGBA_4444;
    case SkBitmap::kARGB_8888_Config:
        return RS_ELEMENT_RGBA_8888;
    case SkBitmap::kRGB_565_Config:
        return RS_ELEMENT_RGB_565;

    default:
        break;
    }
    // If we don't have a conversion mark it as a user type.
    LOGE("Unsupported bitmap type");
    return RS_ELEMENT_USER_U8;
}

static int
nAllocationCreateFromBitmap(JNIEnv *_env, jobject _this, jint dstFmt, jboolean genMips, jobject jbitmap)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    SkBitmap const * nativeBitmap =
            (SkBitmap const *)_env->GetIntField(jbitmap, gNativeBitmapID);
    const SkBitmap& bitmap(*nativeBitmap);
    SkBitmap::Config config = bitmap.getConfig();

    RsElementPredefined e = SkBitmapToPredefined(config);

    if (e != RS_ELEMENT_USER_U8) {
        bitmap.lockPixels();
        const int w = bitmap.width();
        const int h = bitmap.height();
        const void* ptr = bitmap.getPixels();
        jint id = (jint)rsAllocationCreateFromBitmap(w, h, (RsElementPredefined)dstFmt, e, genMips, ptr);
        bitmap.unlockPixels();
        return id;
    }
    return 0;
}


static void
nAllocationDestroy(JNIEnv *_env, jobject _this, jint a)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nAllocationDestroy, con(%p), a(%p)", con, (RsAllocation)a);
    rsAllocationDestroy((RsAllocation)a);
}

static void
nAllocationData_i(JNIEnv *_env, jobject _this, jint alloc, jintArray data)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    jint len = _env->GetArrayLength(data);
    LOG_API("nAllocationData_i, con(%p), alloc(%p), len(%i)", con, (RsAllocation)alloc, len);
    jint *ptr = _env->GetIntArrayElements(data, NULL);
    rsAllocationData((RsAllocation)alloc, ptr);
    _env->ReleaseIntArrayElements(data, ptr, JNI_ABORT);
}

static void
nAllocationData_f(JNIEnv *_env, jobject _this, jint alloc, jfloatArray data)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    jint len = _env->GetArrayLength(data);
    LOG_API("nAllocationData_i, con(%p), alloc(%p), len(%i)", con, (RsAllocation)alloc, len);
    jfloat *ptr = _env->GetFloatArrayElements(data, NULL);
    rsAllocationData((RsAllocation)alloc, ptr);
    _env->ReleaseFloatArrayElements(data, ptr, JNI_ABORT);
}

static void
nAllocationSubData1D_i(JNIEnv *_env, jobject _this, jint alloc, jint offset, jint count, jintArray data)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    jint len = _env->GetArrayLength(data);
    LOG_API("nAllocation1DSubData_i, con(%p), adapter(%p), offset(%i), count(%i), len(%i)", con, (RsAllocation)alloc, offset, count, len);
    jint *ptr = _env->GetIntArrayElements(data, NULL);
    rsAllocation1DSubData((RsAllocation)alloc, offset, count, ptr);
    _env->ReleaseIntArrayElements(data, ptr, JNI_ABORT);
}

static void
nAllocationSubData1D_f(JNIEnv *_env, jobject _this, jint alloc, jint offset, jint count, jfloatArray data)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    jint len = _env->GetArrayLength(data);
    LOG_API("nAllocation1DSubData_f, con(%p), adapter(%p), offset(%i), count(%i), len(%i)", con, (RsAllocation)alloc, offset, count, len);
    jfloat *ptr = _env->GetFloatArrayElements(data, NULL);
    rsAllocation1DSubData((RsAllocation)alloc, offset, count, ptr);
    _env->ReleaseFloatArrayElements(data, ptr, JNI_ABORT);
}

static void
nAllocationSubData2D_i(JNIEnv *_env, jobject _this, jint alloc, jint xoff, jint yoff, jint w, jint h, jintArray data)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    jint len = _env->GetArrayLength(data);
    LOG_API("nAllocation2DSubData_i, con(%p), adapter(%p), xoff(%i), yoff(%i), w(%i), h(%i), len(%i)", con, (RsAllocation)alloc, xoff, yoff, w, h, len);
    jint *ptr = _env->GetIntArrayElements(data, NULL);
    rsAllocation2DSubData((RsAllocation)alloc, xoff, yoff, w, h, ptr);
    _env->ReleaseIntArrayElements(data, ptr, JNI_ABORT);
}

static void
nAllocationSubData2D_f(JNIEnv *_env, jobject _this, jint alloc, jint xoff, jint yoff, jint w, jint h, jfloatArray data)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    jint len = _env->GetArrayLength(data);
    LOG_API("nAllocation2DSubData_i, con(%p), adapter(%p), xoff(%i), yoff(%i), w(%i), h(%i), len(%i)", con, (RsAllocation)alloc, xoff, yoff, w, h, len);
    jfloat *ptr = _env->GetFloatArrayElements(data, NULL);
    rsAllocation2DSubData((RsAllocation)alloc, xoff, yoff, w, h, ptr);
    _env->ReleaseFloatArrayElements(data, ptr, JNI_ABORT);
}



// -----------------------------------

static void
nTriangleMeshDestroy(JNIEnv *_env, jobject _this, jint tm)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nTriangleMeshDestroy, con(%p), tm(%p)", con, (RsAllocation)tm);
    rsTriangleMeshDestroy((RsTriangleMesh)tm);
}

static void
nTriangleMeshBegin(JNIEnv *_env, jobject _this, jint v, jint i)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nTriangleMeshBegin, con(%p), vertex(%p), index(%p)", con, (RsElement)v, (RsElement)i);
    rsTriangleMeshBegin((RsElement)v, (RsElement)i);
}

static void
nTriangleMeshAddVertex_XY(JNIEnv *_env, jobject _this, jfloat x, jfloat y)
{
    float v[] = {x, y};
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nTriangleMeshAddVertex_XY, con(%p), x(%f), y(%f)", con, x, y);
    rsTriangleMeshAddVertex(v);
}

static void
nTriangleMeshAddVertex_XYZ(JNIEnv *_env, jobject _this, jfloat x, jfloat y, jfloat z)
{
    float v[] = {x, y, z};
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nTriangleMeshAddVertex_XYZ, con(%p), x(%f), y(%f), z(%f)", con, x, y, z);
    rsTriangleMeshAddVertex(v);
}

static void
nTriangleMeshAddVertex_XY_ST(JNIEnv *_env, jobject _this, jfloat x, jfloat y, jfloat s, jfloat t)
{
    float v[] = {s, t, x, y};
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nTriangleMeshAddVertex_XY_ST, con(%p), x(%f), y(%f), s(%f), t(%f)", con, x, y, s, t);
    rsTriangleMeshAddVertex(v);
}

static void
nTriangleMeshAddVertex_XYZ_ST(JNIEnv *_env, jobject _this, jfloat x, jfloat y, jfloat z, jfloat s, jfloat t)
{
    float v[] = {s, t, x, y, z};
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nTriangleMeshAddVertex_XYZ_ST, con(%p), x(%f), y(%f), z(%f), s(%f), t(%f)", con, x, y, z, s, t);
    rsTriangleMeshAddVertex(v);
}

static void
nTriangleMeshAddTriangle(JNIEnv *_env, jobject _this, jint i1, jint i2, jint i3)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nTriangleMeshAddTriangle, con(%p), i1(%i), i2(%i), i3(%i)", con, i1, i2, i3);
    rsTriangleMeshAddTriangle(i1, i2, i3);
}

static jint
nTriangleMeshCreate(JNIEnv *_env, jobject _this)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nTriangleMeshCreate, con(%p)", con);
    return (jint) rsTriangleMeshCreate();
}

// -----------------------------------

static void
nAdapter1DDestroy(JNIEnv *_env, jobject _this, jint adapter)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nAdapter1DDestroy, con(%p), adapter(%p)", con, (RsAdapter1D)adapter);
    rsAdapter1DDestroy((RsAdapter1D)adapter);
}

static void
nAdapter1DBindAllocation(JNIEnv *_env, jobject _this, jint adapter, jint alloc)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nAdapter1DBindAllocation, con(%p), adapter(%p), alloc(%p)", con, (RsAdapter1D)adapter, (RsAllocation)alloc);
    rsAdapter1DBindAllocation((RsAdapter1D)adapter, (RsAllocation)alloc);
}

static void
nAdapter1DSetConstraint(JNIEnv *_env, jobject _this, jint adapter, jint dim, jint value)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nAdapter1DSetConstraint, con(%p), adapter(%p), dim(%i), value(%i)", con, (RsAdapter1D)adapter, dim, value);
    rsAdapter1DSetConstraint((RsAdapter1D)adapter, (RsDimension)dim, value);
}

static void
nAdapter1DData_i(JNIEnv *_env, jobject _this, jint adapter, jintArray data)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    jint len = _env->GetArrayLength(data);
    LOG_API("nAdapter1DData_i, con(%p), adapter(%p), len(%i)", con, (RsAdapter1D)adapter, len);
    jint *ptr = _env->GetIntArrayElements(data, NULL);
    rsAdapter1DData((RsAdapter1D)adapter, ptr);
    _env->ReleaseIntArrayElements(data, ptr, 0/*JNI_ABORT*/);
}

static void
nAdapter1DSubData_i(JNIEnv *_env, jobject _this, jint adapter, jint offset, jint count, jintArray data)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    jint len = _env->GetArrayLength(data);
    LOG_API("nAdapter1DSubData_i, con(%p), adapter(%p), offset(%i), count(%i), len(%i)", con, (RsAdapter1D)adapter, offset, count, len);
    jint *ptr = _env->GetIntArrayElements(data, NULL);
    rsAdapter1DSubData((RsAdapter1D)adapter, offset, count, ptr);
    _env->ReleaseIntArrayElements(data, ptr, 0/*JNI_ABORT*/);
}

static void
nAdapter1DData_f(JNIEnv *_env, jobject _this, jint adapter, jfloatArray data)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    jint len = _env->GetArrayLength(data);
    LOG_API("nAdapter1DData_f, con(%p), adapter(%p), len(%i)", con, (RsAdapter1D)adapter, len);
    jfloat *ptr = _env->GetFloatArrayElements(data, NULL);
    rsAdapter1DData((RsAdapter1D)adapter, ptr);
    _env->ReleaseFloatArrayElements(data, ptr, 0/*JNI_ABORT*/);
}

static void
nAdapter1DSubData_f(JNIEnv *_env, jobject _this, jint adapter, jint offset, jint count, jfloatArray data)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    jint len = _env->GetArrayLength(data);
    LOG_API("nAdapter1DSubData_f, con(%p), adapter(%p), offset(%i), count(%i), len(%i)", con, (RsAdapter1D)adapter, offset, count, len);
    jfloat *ptr = _env->GetFloatArrayElements(data, NULL);
    rsAdapter1DSubData((RsAdapter1D)adapter, offset, count, ptr);
    _env->ReleaseFloatArrayElements(data, ptr, 0/*JNI_ABORT*/);
}

static jint
nAdapter1DCreate(JNIEnv *_env, jobject _this)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nAdapter1DCreate, con(%p)", con);
    return (jint)rsAdapter1DCreate();
}

// -----------------------------------

static void
nScriptDestroy(JNIEnv *_env, jobject _this, jint script)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nScriptDestroy, con(%p), script(%p)", con, (RsScript)script);
    rsScriptDestroy((RsScript)script);
}

static void
nScriptBindAllocation(JNIEnv *_env, jobject _this, jint script, jint alloc, jint slot)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nScriptBindAllocation, con(%p), script(%p), alloc(%p), slot(%i)", con, (RsScript)script, (RsAllocation)alloc, slot);
    rsScriptBindAllocation((RsScript)script, (RsAllocation)alloc, slot);
}

static void
nScriptCBegin(JNIEnv *_env, jobject _this)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nScriptCBegin, con(%p)", con);
    rsScriptCBegin();
}

static void
nScriptCSetClearColor(JNIEnv *_env, jobject _this, jfloat r, jfloat g, jfloat b, jfloat a)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nScriptCSetClearColor, con(%p), r(%f), g(%f), b(%f), a(%f)", con, r, g, b, a);
    rsScriptCSetClearColor(r, g, b, a);
}

static void
nScriptCSetClearDepth(JNIEnv *_env, jobject _this, jfloat d)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nScriptCSetClearColor, con(%p), depth(%f)", con, d);
    rsScriptCSetClearDepth(d);
}

static void
nScriptCSetClearStencil(JNIEnv *_env, jobject _this, jint stencil)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nScriptCSetClearStencil, con(%p), stencil(%i)", con, stencil);
    rsScriptCSetClearStencil(stencil);
}

static void
nScriptCAddType(JNIEnv *_env, jobject _this, jint type)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nScriptCAddType, con(%p), type(%p)", con, (RsType)type);
    rsScriptCAddType((RsType)type);
}

static void
nScriptCSetRoot(JNIEnv *_env, jobject _this, jboolean isRoot)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nScriptCSetRoot, con(%p), isRoot(%i)", con, isRoot);
    rsScriptCSetRoot(isRoot);
}

static void
nScriptCSetScript(JNIEnv *_env, jobject _this, jbyteArray scriptRef,
                  jint offset, jint length)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("!!! nScriptCSetScript, con(%p)", con);
    jint _exception = 0;
    jint remaining;
    jbyte* script_base = 0;
    jbyte* script_ptr;
    if (!scriptRef) {
        _exception = 1;
        //_env->ThrowNew(IAEClass, "script == null");
        goto exit;
    }
    if (offset < 0) {
        _exception = 1;
        //_env->ThrowNew(IAEClass, "offset < 0");
        goto exit;
    }
    if (length < 0) {
        _exception = 1;
        //_env->ThrowNew(IAEClass, "length < 0");
        goto exit;
    }
    remaining = _env->GetArrayLength(scriptRef) - offset;
    if (remaining < length) {
        _exception = 1;
        //_env->ThrowNew(IAEClass, "length > script.length - offset");
        goto exit;
    }
    script_base = (jbyte *)
        _env->GetPrimitiveArrayCritical(scriptRef, (jboolean *)0);
    script_ptr = script_base + offset;

    rsScriptCSetText((const char *)script_ptr, length);

exit:
    if (script_base) {
        _env->ReleasePrimitiveArrayCritical(scriptRef, script_base,
                _exception ? JNI_ABORT: 0);
    }
}

static jint
nScriptCCreate(JNIEnv *_env, jobject _this)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nScriptCCreate, con(%p)", con);
    return (jint)rsScriptCCreate();
}

// ---------------------------------------------------------------------------

static void
nProgramFragmentStoreBegin(JNIEnv *_env, jobject _this, jint in, jint out)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nProgramFragmentStoreBegin, con(%p), in(%p), out(%p)", con, (RsElement)in, (RsElement)out);
    rsProgramFragmentStoreBegin((RsElement)in, (RsElement)out);
}

static void
nProgramFragmentStoreDepthFunc(JNIEnv *_env, jobject _this, jint func)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nProgramFragmentStoreDepthFunc, con(%p), func(%i)", con, func);
    rsProgramFragmentStoreDepthFunc((RsDepthFunc)func);
}

static void
nProgramFragmentStoreDepthMask(JNIEnv *_env, jobject _this, jboolean enable)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nProgramFragmentStoreDepthMask, con(%p), enable(%i)", con, enable);
    rsProgramFragmentStoreDepthMask(enable);
}

static void
nProgramFragmentStoreColorMask(JNIEnv *_env, jobject _this, jboolean r, jboolean g, jboolean b, jboolean a)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nProgramFragmentStoreColorMask, con(%p), r(%i), g(%i), b(%i), a(%i)", con, r, g, b, a);
    rsProgramFragmentStoreColorMask(r, g, b, a);
}

static void
nProgramFragmentStoreBlendFunc(JNIEnv *_env, jobject _this, int src, int dst)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nProgramFragmentStoreBlendFunc, con(%p), src(%i), dst(%i)", con, src, dst);
    rsProgramFragmentStoreBlendFunc((RsBlendSrcFunc)src, (RsBlendDstFunc)dst);
}

static void
nProgramFragmentStoreDither(JNIEnv *_env, jobject _this, jboolean enable)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nProgramFragmentStoreDither, con(%p), enable(%i)", con, enable);
    rsProgramFragmentStoreDither(enable);
}

static jint
nProgramFragmentStoreCreate(JNIEnv *_env, jobject _this)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nProgramFragmentStoreCreate, con(%p)", con);

    return (jint)rsProgramFragmentStoreCreate();
}

static void
nProgramFragmentStoreDestroy(JNIEnv *_env, jobject _this, jint pgm)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nProgramFragmentStoreDestroy, con(%p), pgm(%i)", con, pgm);
    rsProgramFragmentStoreDestroy((RsProgramFragmentStore)pgm);
}

// ---------------------------------------------------------------------------

static void
nProgramFragmentBegin(JNIEnv *_env, jobject _this, jint in, jint out)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nProgramFragmentBegin, con(%p), in(%p), out(%p)", con, (RsElement)in, (RsElement)out);
    rsProgramFragmentBegin((RsElement)in, (RsElement)out);
}

static void
nProgramFragmentBindTexture(JNIEnv *_env, jobject _this, jint vpf, jint slot, jint a)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nProgramFragmentBindTexture, con(%p), vpf(%p), slot(%i), a(%p)", con, (RsProgramFragment)vpf, slot, (RsAllocation)a);
    rsProgramFragmentBindTexture((RsProgramFragment)vpf, slot, (RsAllocation)a);
}

static void
nProgramFragmentBindSampler(JNIEnv *_env, jobject _this, jint vpf, jint slot, jint a)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nProgramFragmentBindSampler, con(%p), vpf(%p), slot(%i), a(%p)", con, (RsProgramFragment)vpf, slot, (RsSampler)a);
    rsProgramFragmentBindSampler((RsProgramFragment)vpf, slot, (RsSampler)a);
}

static void
nProgramFragmentSetType(JNIEnv *_env, jobject _this, jint slot, jint vt)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nProgramFragmentSetType, con(%p), slot(%i), vt(%p)", con, slot, (RsType)vt);
    rsProgramFragmentSetType(slot, (RsType)vt);
}

static void
nProgramFragmentSetEnvMode(JNIEnv *_env, jobject _this, jint slot, jint env)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nProgramFragmentSetEnvMode, con(%p), slot(%i), vt(%i)", con, slot, env);
    rsProgramFragmentSetEnvMode(slot, (RsTexEnvMode)env);
}

static void
nProgramFragmentSetTexEnable(JNIEnv *_env, jobject _this, jint slot, jboolean enable)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nProgramFragmentSetTexEnable, con(%p), slot(%i), enable(%i)", con, slot, enable);
    rsProgramFragmentSetTexEnable(slot, enable);
}

static jint
nProgramFragmentCreate(JNIEnv *_env, jobject _this, jint slot, jboolean enable)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nProgramFragmentCreate, con(%p)", con);
    return (jint)rsProgramFragmentCreate();
}

static void
nProgramFragmentDestroy(JNIEnv *_env, jobject _this, jint pgm)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nProgramFragmentDestroy, con(%p), pgm(%i)", con, pgm);
    rsProgramFragmentDestroy((RsProgramFragment)pgm);
}


// ---------------------------------------------------------------------------

static void
nContextBindRootScript(JNIEnv *_env, jobject _this, jint script)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nContextBindRootScript, con(%p), script(%p)", con, (RsScript)script);
    rsContextBindRootScript((RsScript)script);
}

static void
nContextBindProgramFragmentStore(JNIEnv *_env, jobject _this, jint pfs)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nContextBindProgramFragmentStore, con(%p), pfs(%p)", con, (RsProgramFragmentStore)pfs);
    rsContextBindProgramFragmentStore((RsProgramFragmentStore)pfs);
}

static void
nContextBindProgramFragment(JNIEnv *_env, jobject _this, jint pf)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nContextBindProgramFragment, con(%p), pf(%p)", con, (RsProgramFragment)pf);
    rsContextBindProgramFragment((RsProgramFragment)pf);
}

// ---------------------------------------------------------------------------

static void
nSamplerDestroy(JNIEnv *_env, jobject _this, jint s)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nSamplerDestroy, con(%p), sampler(%p)", con, (RsSampler)s);
    rsSamplerDestroy((RsSampler)s);
}

static void
nSamplerBegin(JNIEnv *_env, jobject _this)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nSamplerBegin, con(%p)", con);
    rsSamplerBegin();
}

static void
nSamplerSet(JNIEnv *_env, jobject _this, jint p, jint v)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nSamplerSet, con(%p), param(%i), value(%i)", con, p, v);
    rsSamplerSet((RsSamplerParam)p, (RsSamplerValue)v);
}

static jint
nSamplerCreate(JNIEnv *_env, jobject _this)
{
    RsContext con = (RsContext)(_env->GetIntField(_this, gContextId));
    LOG_API("nSamplerCreate, con(%p), script(%p)", con, (RsScript)script);
    return (jint)rsSamplerCreate();
}


// ---------------------------------------------------------------------------


static const char *classPathName = "android/renderscript/RenderScript";

static JNINativeMethod methods[] = {
{"_nInit",                         "()V",                                  (void*)_nInit },
{"nDeviceCreate",                  "()I",                                  (void*)nDeviceCreate },
{"nDeviceDestroy",                 "(I)V",                                 (void*)nDeviceDestroy },
{"nContextCreate",                 "(ILandroid/view/Surface;I)I",          (void*)nContextCreate },
{"nContextDestroy",                "(I)V",                                 (void*)nContextDestroy },
{"nAssignName",                    "(I[B)V",                               (void*)nAssignName },

{"nElementBegin",                  "()V",                                  (void*)nElementBegin },
{"nElementAddPredefined",          "(I)V",                                 (void*)nElementAddPredefined },
{"nElementAdd",                    "(IIII)V",                              (void*)nElementAdd },
{"nElementCreate",                 "()I",                                  (void*)nElementCreate },
{"nElementGetPredefined",          "(I)I",                                 (void*)nElementGetPredefined },
{"nElementDestroy",                "(I)V",                                 (void*)nElementDestroy },

{"nTypeBegin",                     "(I)V",                                 (void*)nTypeBegin },
{"nTypeAdd",                       "(II)V",                                (void*)nTypeAdd },
{"nTypeCreate",                    "()I",                                  (void*)nTypeCreate },
{"nTypeDestroy",                   "(I)V",                                 (void*)nTypeDestroy },

{"nAllocationCreateTyped",         "(I)I",                                 (void*)nAllocationCreateTyped },
{"nAllocationCreatePredefSized",   "(II)I",                                (void*)nAllocationCreatePredefSized },
{"nAllocationCreateSized",         "(II)I",                                (void*)nAllocationCreateSized },
{"nAllocationCreateFromBitmap",    "(IZLandroid/graphics/Bitmap;)I",       (void*)nAllocationCreateFromBitmap },
{"nAllocationUploadToTexture",     "(II)V",                                (void*)nAllocationUploadToTexture },
{"nAllocationDestroy",             "(I)V",                                 (void*)nAllocationDestroy },
{"nAllocationData",                "(I[I)V",                               (void*)nAllocationData_i },
{"nAllocationData",                "(I[F)V",                               (void*)nAllocationData_f },
{"nAllocationSubData1D",           "(III[I)V",                             (void*)nAllocationSubData1D_i },
{"nAllocationSubData1D",           "(III[F)V",                             (void*)nAllocationSubData1D_f },
{"nAllocationSubData2D",           "(IIIII[I)V",                           (void*)nAllocationSubData2D_i },
{"nAllocationSubData2D",           "(IIIII[F)V",                           (void*)nAllocationSubData2D_f },

{"nTriangleMeshDestroy",           "(I)V",                                 (void*)nTriangleMeshDestroy },
{"nTriangleMeshBegin",             "(II)V",                                (void*)nTriangleMeshBegin },
{"nTriangleMeshAddVertex_XY",      "(FF)V",                                (void*)nTriangleMeshAddVertex_XY },
{"nTriangleMeshAddVertex_XYZ",     "(FFF)V",                               (void*)nTriangleMeshAddVertex_XYZ },
{"nTriangleMeshAddVertex_XY_ST",   "(FFFF)V",                              (void*)nTriangleMeshAddVertex_XY_ST },
{"nTriangleMeshAddVertex_XYZ_ST",  "(FFFFF)V",                             (void*)nTriangleMeshAddVertex_XYZ_ST },
{"nTriangleMeshAddTriangle",       "(III)V",                               (void*)nTriangleMeshAddTriangle },
{"nTriangleMeshCreate",            "()I",                                  (void*)nTriangleMeshCreate },

{"nAdapter1DDestroy",              "(I)V",                                 (void*)nAdapter1DDestroy },
{"nAdapter1DBindAllocation",       "(II)V",                                (void*)nAdapter1DBindAllocation },
{"nAdapter1DSetConstraint",        "(III)V",                               (void*)nAdapter1DSetConstraint },
{"nAdapter1DData",                 "(I[I)V",                               (void*)nAdapter1DData_i },
{"nAdapter1DSubData",              "(III[I)V",                             (void*)nAdapter1DSubData_i },
{"nAdapter1DData",                 "(I[F)V",                               (void*)nAdapter1DData_f },
{"nAdapter1DSubData",              "(III[F)V",                             (void*)nAdapter1DSubData_f },
{"nAdapter1DCreate",               "()I",                                  (void*)nAdapter1DCreate },

{"nScriptDestroy",                 "(I)V",                                 (void*)nScriptDestroy },
{"nScriptBindAllocation",          "(III)V",                               (void*)nScriptBindAllocation },
{"nScriptCBegin",                  "()V",                                  (void*)nScriptCBegin },
{"nScriptCSetClearColor",          "(FFFF)V",                              (void*)nScriptCSetClearColor },
{"nScriptCSetClearDepth",          "(F)V",                                 (void*)nScriptCSetClearDepth },
{"nScriptCSetClearStencil",        "(I)V",                                 (void*)nScriptCSetClearStencil },
{"nScriptCAddType",                "(I)V",                                 (void*)nScriptCAddType },
{"nScriptCSetRoot",                "(Z)V",                                 (void*)nScriptCSetRoot },
{"nScriptCSetScript",              "([BII)V",                              (void*)nScriptCSetScript },
{"nScriptCCreate",                 "()I",                                  (void*)nScriptCCreate },

{"nProgramFragmentStoreBegin",     "(II)V",                                (void*)nProgramFragmentStoreBegin },
{"nProgramFragmentStoreDepthFunc", "(I)V",                                 (void*)nProgramFragmentStoreDepthFunc },
{"nProgramFragmentStoreDepthMask", "(Z)V",                                 (void*)nProgramFragmentStoreDepthMask },
{"nProgramFragmentStoreColorMask", "(ZZZZ)V",                              (void*)nProgramFragmentStoreColorMask },
{"nProgramFragmentStoreBlendFunc", "(II)V",                                (void*)nProgramFragmentStoreBlendFunc },
{"nProgramFragmentStoreDither",    "(Z)V",                                 (void*)nProgramFragmentStoreDither },
{"nProgramFragmentStoreCreate",    "()I",                                  (void*)nProgramFragmentStoreCreate },
{"nProgramFragmentStoreDestroy",   "(I)V",                                 (void*)nProgramFragmentStoreDestroy },

{"nProgramFragmentBegin",          "(II)V",                                (void*)nProgramFragmentBegin },
{"nProgramFragmentBindTexture",    "(III)V",                               (void*)nProgramFragmentBindTexture },
{"nProgramFragmentBindSampler",    "(III)V",                               (void*)nProgramFragmentBindSampler },
{"nProgramFragmentSetType",        "(II)V",                                (void*)nProgramFragmentSetType },
{"nProgramFragmentSetEnvMode",     "(II)V",                                (void*)nProgramFragmentSetEnvMode },
{"nProgramFragmentSetTexEnable",   "(IZ)V",                                (void*)nProgramFragmentSetTexEnable },
{"nProgramFragmentCreate",         "()I",                                  (void*)nProgramFragmentCreate },
{"nProgramFragmentDestroy",        "(I)V",                                 (void*)nProgramFragmentDestroy },

{"nContextBindRootScript",         "(I)V",                                 (void*)nContextBindRootScript },
{"nContextBindProgramFragmentStore","(I)V",                                (void*)nContextBindProgramFragmentStore },
{"nContextBindProgramFragment",    "(I)V",                                 (void*)nContextBindProgramFragment },

{"nSamplerDestroy",                "(I)V",                                 (void*)nSamplerDestroy },
{"nSamplerBegin",                  "()V",                                  (void*)nSamplerBegin },
{"nSamplerSet",                    "(II)V",                                (void*)nSamplerSet },
{"nSamplerCreate",                 "()I",                                  (void*)nSamplerCreate },

};

static int registerFuncs(JNIEnv *_env)
{
    return android::AndroidRuntime::registerNativeMethods(
            _env, classPathName, methods, NELEM(methods));
}

// ---------------------------------------------------------------------------

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    LOGE("****************************************************\n");

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (registerFuncs(env) < 0) {
        LOGE("ERROR: MediaPlayer native registration failed\n");
        goto bail;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;

bail:
    return result;
}
