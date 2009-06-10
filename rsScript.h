/*
 * Copyright (C) 2009 The Android Open Source Project
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

#ifndef ANDROID_RS_SCRIPT_H
#define ANDROID_RS_SCRIPT_H

#include "rsAllocation.h"


// ---------------------------------------------------------------------------
namespace android {
namespace renderscript {

class ProgramVertex;
class ProgramFragment;
class ProgramRaster;
class ProgramFragmentStore;

class Script : public ObjectBase
{
public:

    Script();
    virtual ~Script();


    struct Enviroment_t {
        bool mIsRoot;
        bool mIsOrtho;
        float mClearColor[4];
        float mClearDepth;
        uint32_t mClearStencil;

        ObjectBaseRef<ProgramVertex> mVertex;
        ObjectBaseRef<ProgramFragment> mFragment;
        //ObjectBaseRef<ProgramRaster> mRaster;
        ObjectBaseRef<ProgramFragmentStore> mFragmentStore;

    };
    Enviroment_t mEnviroment;

    const Type * mConstantBufferTypes;
    uint32_t mCounstantBufferCount;

    ObjectBaseRef<Allocation> mSlots[16];

    virtual bool run(Context *, uint32_t launchID) = 0;
};



}
}
#endif

