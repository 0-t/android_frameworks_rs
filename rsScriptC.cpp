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

#include "rsContext.h"
#include "rsScriptC.h"
#include "rsMatrix.h"

#include "acc/acc.h"
#include "utils/Timers.h"

#include <GLES/gl.h>
#include <GLES/glext.h>

using namespace android;
using namespace android::renderscript;

#define GET_TLS()  Context::ScriptTLSStruct * tls = \
    (Context::ScriptTLSStruct *)pthread_getspecific(Context::gThreadTLSKey); \
    Context * rsc = tls->mContext; \
    ScriptC * sc = (ScriptC *) tls->mScript


ScriptC::ScriptC()
{
    mAccScript = NULL;
    memset(&mProgram, 0, sizeof(mProgram));
}

ScriptC::~ScriptC()
{
    if (mAccScript) {
        accDeleteScript(mAccScript);
    }
}


bool ScriptC::run(Context *rsc, uint32_t launchIndex)
{
    Context::ScriptTLSStruct * tls =
    (Context::ScriptTLSStruct *)pthread_getspecific(Context::gThreadTLSKey);

    if (mEnviroment.mFragmentStore.get()) {
        rsc->setFragmentStore(mEnviroment.mFragmentStore.get());
    }
    if (mEnviroment.mFragment.get()) {
        rsc->setFragment(mEnviroment.mFragment.get());
    }
    if (mEnviroment.mVertex.get()) {
        rsc->setVertex(mEnviroment.mVertex.get());
    }

    if (launchIndex == 0) {
        mEnviroment.mStartTimeMillis
                = nanoseconds_to_milliseconds(systemTime(SYSTEM_TIME_MONOTONIC));
    }

    bool ret = false;
    tls->mScript = this;
    ret = mProgram.mScript(launchIndex) != 0;
    tls->mScript = NULL;
    return ret;
}

ScriptCState::ScriptCState()
{
    clear();
}

ScriptCState::~ScriptCState()
{
    if (mAccScript) {
        accDeleteScript(mAccScript);
    }
}

void ScriptCState::clear()
{
    memset(&mProgram, 0, sizeof(mProgram));

    for (uint32_t ct=0; ct < MAX_SCRIPT_BANKS; ct++) {
        mConstantBufferTypes[ct].clear();
        mSlotNames[ct].setTo("");
    }

    memset(&mEnviroment, 0, sizeof(mEnviroment));
    mEnviroment.mClearColor[0] = 0;
    mEnviroment.mClearColor[1] = 0;
    mEnviroment.mClearColor[2] = 0;
    mEnviroment.mClearColor[3] = 1;
    mEnviroment.mClearDepth = 1;
    mEnviroment.mClearStencil = 0;
    mEnviroment.mIsRoot = false;

    mAccScript = NULL;

    mInt32Defines.clear();
    mFloatDefines.clear();
}

static ACCvoid* symbolLookup(ACCvoid* pContext, const ACCchar* name)
{
    const ScriptCState::SymbolTable_t *sym = ScriptCState::lookupSymbol(name);
    if (sym) {
        return sym->mPtr;
    }
    LOGE("ScriptC sym lookup failed for %s", name);
    return NULL;
}

void ScriptCState::runCompiler(Context *rsc)
{
    mAccScript = accCreateScript();
    String8 tmp;

    rsc->appendNameDefines(&tmp);
    appendDecls(&tmp);
    rsc->appendVarDefines(&tmp);
    appendVarDefines(&tmp);
    appendTypes(&tmp);
    tmp.append("#line 1\n");

    const char* scriptSource[] = {tmp.string(), mProgram.mScriptText};
    int scriptLength[] = {tmp.length(), mProgram.mScriptTextLength} ;
    accScriptSource(mAccScript, sizeof(scriptLength) / sizeof(int), scriptSource, scriptLength);
    accRegisterSymbolCallback(mAccScript, symbolLookup, NULL);
    accCompileScript(mAccScript);
    accGetScriptLabel(mAccScript, "main", (ACCvoid**) &mProgram.mScript);
    rsAssert(mProgram.mScript);

    if (!mProgram.mScript) {
        ACCchar buf[4096];
        ACCsizei len;
        accGetScriptInfoLog(mAccScript, sizeof(buf), &len, buf);
        LOGE(buf);
    }

    mEnviroment.mFragment.set(rsc->getDefaultProgramFragment());
    mEnviroment.mVertex.set(rsc->getDefaultProgramVertex());
    mEnviroment.mFragmentStore.set(rsc->getDefaultProgramFragmentStore());

    if (mProgram.mScript) {
        const static int pragmaMax = 16;
        ACCsizei pragmaCount;
        ACCchar * str[pragmaMax];
        accGetPragmas(mAccScript, &pragmaCount, pragmaMax, &str[0]);

        for (int ct=0; ct < pragmaCount; ct+=2) {
            if (!strcmp(str[ct], "version")) {
                continue;
            }

            if (!strcmp(str[ct], "stateVertex")) {
                if (!strcmp(str[ct+1], "default")) {
                    continue;
                }
                if (!strcmp(str[ct+1], "parent")) {
                    mEnviroment.mVertex.clear();
                    continue;
                }
                ProgramVertex * pv = (ProgramVertex *)rsc->lookupName(str[ct+1]);
                if (pv != NULL) {
                    mEnviroment.mVertex.set(pv);
                    continue;
                }
                LOGE("Unreconized value %s passed to stateVertex", str[ct+1]);
            }

            if (!strcmp(str[ct], "stateRaster")) {
                LOGE("Unreconized value %s passed to stateRaster", str[ct+1]);
            }

            if (!strcmp(str[ct], "stateFragment")) {
                if (!strcmp(str[ct+1], "default")) {
                    continue;
                }
                if (!strcmp(str[ct+1], "parent")) {
                    mEnviroment.mFragment.clear();
                    continue;
                }
                ProgramFragment * pf = (ProgramFragment *)rsc->lookupName(str[ct+1]);
                if (pf != NULL) {
                    mEnviroment.mFragment.set(pf);
                    continue;
                }
                LOGE("Unreconized value %s passed to stateFragment", str[ct+1]);
            }

            if (!strcmp(str[ct], "stateFragmentStore")) {
                if (!strcmp(str[ct+1], "default")) {
                    continue;
                }
                if (!strcmp(str[ct+1], "parent")) {
                    mEnviroment.mFragmentStore.clear();
                    continue;
                }
                ProgramFragmentStore * pfs =
                    (ProgramFragmentStore *)rsc->lookupName(str[ct+1]);
                if (pfs != NULL) {
                    mEnviroment.mFragmentStore.set(pfs);
                    continue;
                }
                LOGE("Unreconized value %s passed to stateFragmentStore", str[ct+1]);
            }

        }


    } else {
        // Deal with an error.
    }
}


void ScriptCState::appendVarDefines(String8 *str)
{
    char buf[256];
    LOGD("appendVarDefines mInt32Defines.size()=%d mFloatDefines.size()=%d\n",
            mInt32Defines.size(), mFloatDefines.size());
    for (size_t ct=0; ct < mInt32Defines.size(); ct++) {
        str->append("#define ");
        str->append(mInt32Defines.keyAt(ct));
        str->append(" ");
        sprintf(buf, "%i\n", (int)mInt32Defines.valueAt(ct));
        str->append(buf);
    }
    for (size_t ct=0; ct < mFloatDefines.size(); ct++) {
        str->append("#define ");
        str->append(mFloatDefines.keyAt(ct));
        str->append(" ");
        sprintf(buf, "%ff\n", mFloatDefines.valueAt(ct));
        str->append(buf);
    }
}

void ScriptCState::appendTypes(String8 *str)
{
    char buf[256];
    String8 tmp;

    for (size_t ct=0; ct < MAX_SCRIPT_BANKS; ct++) {
        const Type *t = mConstantBufferTypes[ct].get();
        if (!t) {
            continue;
        }
        const Element *e = t->getElement();

        if (t->getName()) {
            for (size_t ct2=0; ct2 < e->getComponentCount(); ct2++) {
                const Component *c = e->getComponent(ct2);
                tmp.setTo("#define OFFSETOF_");
                tmp.append(t->getName());
                tmp.append("_");
                tmp.append(c->getComponentName());
                sprintf(buf, " %i\n", ct2);
                tmp.append(buf);
                LOGD(tmp);
                str->append(tmp);
            }
        }

        if (mSlotNames[ct].length() > 0) {
            for (size_t ct2=0; ct2 < e->getComponentCount(); ct2++) {
                const Component *c = e->getComponent(ct2);
                tmp.setTo("#define ");
                tmp.append(mSlotNames[ct]);
                tmp.append("_");
                tmp.append(c->getComponentName());
                switch (c->getType()) {
                case Component::FLOAT:
                    tmp.append(" loadF(");
                    break;
                case Component::SIGNED:
                    sprintf(buf, " loadI%i(", c->getBits());
                    tmp.append(buf);
                    break;
                case Component::UNSIGNED:
                    sprintf(buf, " loadU%i(", c->getBits());
                    tmp.append(buf);
                    break;
                }
                sprintf(buf, "%i, %i)\n", ct, ct2);
                tmp.append(buf);

                LOGD(tmp);
                str->append(tmp);
            }
        }
    }

}


namespace android {
namespace renderscript {

void rsi_ScriptCBegin(Context * rsc)
{
    ScriptCState *ss = &rsc->mScriptC;
    ss->clear();
}

void rsi_ScriptCSetScript(Context * rsc, void *vp)
{
    ScriptCState *ss = &rsc->mScriptC;
    ss->mProgram.mScript = reinterpret_cast<ScriptC::RunScript_t>(vp);
}

void rsi_ScriptCSetText(Context *rsc, const char *text, uint32_t len)
{
    ScriptCState *ss = &rsc->mScriptC;
    ss->mProgram.mScriptText = text;
    ss->mProgram.mScriptTextLength = len;
}


RsScript rsi_ScriptCCreate(Context * rsc)
{
    ScriptCState *ss = &rsc->mScriptC;

    ss->runCompiler(rsc);

    ScriptC *s = new ScriptC();
    s->incRef();
    s->mAccScript = ss->mAccScript;
    ss->mAccScript = NULL;
    s->mEnviroment = ss->mEnviroment;
    s->mProgram = ss->mProgram;
    for (int ct=0; ct < MAX_SCRIPT_BANKS; ct++) {
        s->mTypes[ct].set(ss->mConstantBufferTypes[ct].get());
        s->mSlotNames[ct] = ss->mSlotNames[ct];
    }

    ss->clear();
    return s;
}

void rsi_ScriptCSetDefineF(Context *rsc, const char* name, float value)
{
    ScriptCState *ss = &rsc->mScriptC;
    ss->mFloatDefines.add(String8(name), value);
}

void rsi_ScriptCSetDefineI32(Context *rsc, const char* name, int32_t value)
{
    ScriptCState *ss = &rsc->mScriptC;
    ss->mInt32Defines.add(String8(name), value);
}

}
}


