/*
 * Copyright (C) 2012 The Android Open Source Project
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


#include "rsdCore.h"
#include "rsdIntrinsics.h"
#include "rsdAllocation.h"

using namespace android;
using namespace android::renderscript;

enum IntrinsicEnums {
    INTRINSIC_UNDEFINED,
    INTRINSIC_CONVOLVE_3x3,
    INTRINXIC_COLORMATRIX

};


void * rsdIntrinsic_InitConvolve3x3(const android::renderscript::Context *dc,
                                    android::renderscript::Script *script,
                                    RsdIntriniscFuncs_t *funcs);


void * rsdIntrinsic_Init(const android::renderscript::Context *dc,
                       android::renderscript::Script *script,
                       RsScriptIntrinsicID iid,
                       RsdIntriniscFuncs_t *funcs) {

    switch(iid) {
    case INTRINSIC_CONVOLVE_3x3:
        return rsdIntrinsic_InitConvolve3x3(dc, script, funcs);

    default:
        return NULL;
    }
    return NULL;
}



