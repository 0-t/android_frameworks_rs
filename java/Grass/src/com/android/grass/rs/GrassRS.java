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

package com.android.grass.rs;

import android.content.res.Resources;
import static android.renderscript.RenderScript.SamplerParam.*;
import static android.renderscript.RenderScript.SamplerValue.*;
import static android.renderscript.RenderScript.EnvMode.*;
import static android.renderscript.RenderScript.DepthFunc.*;
import static android.renderscript.RenderScript.BlendSrcFunc;
import static android.renderscript.RenderScript.BlendDstFunc;
import android.renderscript.RenderScript;
import android.renderscript.Element;
import android.renderscript.Allocation;
import android.renderscript.ProgramVertexAlloc;
import static android.renderscript.Element.*;

import static android.util.MathUtils.*;

import java.util.TimeZone;

class GrassRS {
    private static final int RSID_STATE = 0;
    private static final int RSID_STATE_FRAMECOUNT = 0;
    private static final int RSID_STATE_BLADES_COUNT = 1;

    private static final int RSID_SKY_TEXTURES = 1;
    private static final int SKY_TEXTURES_COUNT = 4;
    
    private static final int RSID_BLADES = 2;    
    private static final int BLADES_COUNT = 100;
    private static final int BLADE_STRUCT_FIELDS_COUNT = 12;
    private static final int BLADE_STRUCT_DEGREE = 0;
    private static final int BLADE_STRUCT_SIZE = 1;
    private static final int BLADE_STRUCT_XPOS = 2;
    private static final int BLADE_STRUCT_YPOS = 3;
    private static final int BLADE_STRUCT_OFFSET = 4;
    private static final int BLADE_STRUCT_SCALE = 5;
    private static final int BLADE_STRUCT_LENGTHX = 6;
    private static final int BLADE_STRUCT_LENGTHY = 7;
    private static final int BLADE_STRUCT_HARDNESS = 8;
    private static final int BLADE_STRUCT_H = 9;
    private static final int BLADE_STRUCT_S = 10;
    private static final int BLADE_STRUCT_B = 11;
    
    private Resources mResources;
    private RenderScript mRS;
    
    private final int mWidth;
    private final int mHeight;

    @SuppressWarnings({"FieldCanBeLocal"})
    private RenderScript.Script mScript;
    @SuppressWarnings({"FieldCanBeLocal"})
    private RenderScript.Sampler mSampler;
    @SuppressWarnings({"FieldCanBeLocal"})
    private RenderScript.ProgramFragment mPfBackground;
    @SuppressWarnings({"FieldCanBeLocal"})
    private RenderScript.ProgramFragmentStore mPfsBackground;
    @SuppressWarnings({"FieldCanBeLocal"})
    private RenderScript.ProgramVertex mPvBackground;    
    @SuppressWarnings({"FieldCanBeLocal"})
    private ProgramVertexAlloc mPvOrthoAlloc;

    @SuppressWarnings({"FieldCanBeLocal"})
    private Allocation mSkyTexturesIDs;
    @SuppressWarnings({"FieldCanBeLocal"})
    private Allocation[] mSkyTextures;
    @SuppressWarnings({"FieldCanBeLocal"})
    private int[] mSkyBufferIDs;
    @SuppressWarnings({"FieldCanBeLocal"})
    private Allocation mState;
    @SuppressWarnings({"FieldCanBeLocal"})
    private Allocation mBlades;

    public GrassRS(int width, int height) {
        mWidth = width;
        mHeight = height;
    }

    public void init(RenderScript rs, Resources res) {
        mRS = rs;
        mResources = res;
        initRS();
    }

    private void initRS() {
        createProgramVertex();
        createProgramFragmentStore();
        createProgramFragment();
        createScriptStructures();

        mRS.scriptCBegin();
        mRS.scriptCSetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        mRS.scriptCSetScript(mResources, R.raw.grass);
        mRS.scriptCSetTimeZone(TimeZone.getDefault().getID());
        mRS.scriptCSetRoot(true);

        mScript = mRS.scriptCCreate();

        loadSkyTextures();
        mScript.bindAllocation(mState, RSID_STATE);
        mScript.bindAllocation(mSkyTexturesIDs, RSID_SKY_TEXTURES);
        mScript.bindAllocation(mBlades, RSID_BLADES);

        mRS.contextBindRootScript(mScript);
    }

    private void createScriptStructures() {
        final int[] data = new int[2];
        mState = Allocation.createSized(mRS, USER_I32, data.length);
        data[RSID_STATE_FRAMECOUNT] = 0;
        data[RSID_STATE_BLADES_COUNT] = BLADES_COUNT;
        mState.data(data);

        final float[] blades = new float[BLADES_COUNT * BLADE_STRUCT_FIELDS_COUNT];
        mBlades = Allocation.createSized(mRS, USER_FLOAT, blades.length);
        for (int i = 0; i < blades.length; i+= BLADE_STRUCT_FIELDS_COUNT) {
            createBlade(blades, i);
        }
        mBlades.data(blades);
    }

    private void createBlade(float[] blades, int index) {
        //noinspection PointlessArithmeticExpression
        blades[index + BLADE_STRUCT_DEGREE] = 0.0f;
        blades[index + BLADE_STRUCT_SIZE] = random(4.0f) + 4.0f;
        blades[index + BLADE_STRUCT_XPOS] = random(mWidth);
        blades[index + BLADE_STRUCT_YPOS] = mHeight;
        blades[index + BLADE_STRUCT_OFFSET] = random(0.2f) - 0.1f;
        blades[index + BLADE_STRUCT_SCALE] = random(0.6f) + 0.2f;
        blades[index + BLADE_STRUCT_LENGTHX] = random(4.5f) + 3.0f;
        blades[index + BLADE_STRUCT_LENGTHY] = random(5.5f) + 2.0f;
        blades[index + BLADE_STRUCT_HARDNESS] = random(1.0f) + 0.2f;
        blades[index + BLADE_STRUCT_H] = (51.0f + random(5.0f)) / 255.0f;
        blades[index + BLADE_STRUCT_S] = (200.0f + random(55.0f)) / 255.0f;
        blades[index + BLADE_STRUCT_B] = (90.0f + random(165.0f)) / 255.0f;
    }

    private void loadSkyTextures() {
        mSkyBufferIDs = new int[SKY_TEXTURES_COUNT];
        mSkyTextures = new Allocation[SKY_TEXTURES_COUNT];
        mSkyTexturesIDs = Allocation.createSized(mRS, USER_FLOAT, SKY_TEXTURES_COUNT);

        final Allocation[] textures = mSkyTextures;
        textures[0] = loadTexture(R.drawable.night, "night");
        textures[1] = loadTexture(R.drawable.sunrise, "sunrise");
        textures[2] = loadTexture(R.drawable.sky, "sky");
        textures[3] = loadTexture(R.drawable.sunset, "sunset");

        final int[] bufferIds = mSkyBufferIDs;
        final int count = textures.length;

        for (int i = 0; i < count; i++) {
            final Allocation texture = textures[i];
            texture.uploadToTexture(0);
            bufferIds[i] = texture.getID();
        }

        mSkyTexturesIDs.data(bufferIds);
    }

    private Allocation loadTexture(int id, String name) {
        Allocation allocation = Allocation.createFromBitmapResource(mRS, mResources, id,
                Element.RGB_565, false);
        allocation.setName(name);
        return allocation;
    }

    private void createProgramFragment() {
        mRS.samplerBegin();
        mRS.samplerSet(FILTER_MIN, LINEAR);
        mRS.samplerSet(FILTER_MAG, LINEAR);
        mRS.samplerSet(WRAP_MODE_S, CLAMP);
        mRS.samplerSet(WRAP_MODE_T, CLAMP);
        mSampler = mRS.samplerCreate();

        mRS.programFragmentBegin(null, null);
        mRS.programFragmentSetTexEnable(0, true);
        mRS.programFragmentSetTexEnvMode(0, REPLACE);
        mPfBackground = mRS.programFragmentCreate();
        mPfBackground.setName("PFBackground");
        mPfBackground.bindSampler(mSampler, 0);
    }

    private void createProgramFragmentStore() {
        mRS.programFragmentStoreBegin(null, null);
        mRS.programFragmentStoreDepthFunc(ALWAYS);
        mRS.programFragmentStoreBlendFunc(BlendSrcFunc.SRC_ALPHA, BlendDstFunc.ONE_MINUS_SRC_ALPHA);
        mRS.programFragmentStoreDitherEnable(true);
        mRS.programFragmentStoreDepthMask(false);
        mPfsBackground = mRS.programFragmentStoreCreate();
        mPfsBackground.setName("PFSBackground");
    }

    private void createProgramVertex() {
        mPvOrthoAlloc = new ProgramVertexAlloc(mRS);
        mPvOrthoAlloc.setupOrthoWindow(mWidth, mHeight);

        mRS.programVertexBegin(null, null);
        mRS.programVertexSetTextureMatrixEnable(true);
        mPvBackground = mRS.programVertexCreate();
        mPvBackground.bindAllocation(0, mPvOrthoAlloc.mAlloc);
        mPvBackground.setName("PVBackground");
    }
}
