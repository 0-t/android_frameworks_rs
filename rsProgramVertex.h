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

#ifndef ANDROID_RS_PROGRAM_VERTEX_H
#define ANDROID_RS_PROGRAM_VERTEX_H

#include "rsProgram.h"

// ---------------------------------------------------------------------------
namespace android {
namespace renderscript {


class ProgramVertex : public Program
{
public:
    const static uint32_t MAX_CONSTANTS = 2;

    ProgramVertex(Element *in, Element *out);
    virtual ~ProgramVertex();

    virtual void setupGL();


    void setConstantType(uint32_t slot, const Type *);
    void bindAllocation(uint32_t slot, Allocation *);
    void setTextureMatrixEnable(bool e) {mTextureMatrixEnable = e;}
    void setProjectionEnabled(bool e) {mProjectionEnable = e;}
    void setTransformEnable(bool e) {mTransformEnable = e;}
    void setProjectionEnable(bool e) {mProjectionEnable = e;}

protected:
    bool mDirty;

    ObjectBaseRef<Allocation> mConstants[MAX_CONSTANTS];
    ObjectBaseRef<const Type> mConstantTypes[MAX_CONSTANTS];

    // Hacks to create a program for now
    bool mTextureMatrixEnable;
    bool mProjectionEnable;
    bool mTransformEnable;

};


class ProgramVertexState 
{
public:
    ProgramVertexState();
    ~ProgramVertexState();

    ProgramVertex *mPV;

    //ObjectBaseRef<Type> mTextureTypes[ProgramFragment::MAX_TEXTURE];


};


}
}
#endif


