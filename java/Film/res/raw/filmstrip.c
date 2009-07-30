// Fountain test script

#pragma version(1)
#pragma stateVertex(PVBackground)
#pragma stateFragment(PFBackground)
#pragma stateFragmentStore(PFSBackground)

/*
typedef struct FilmScriptUserEnvRec {
    RsAllocation tex[13];
    int32_t triangleOffsets[64];
    float triangleOffsetsTex[64];
    int32_t triangleOffsetsCount;
} FilmScriptUserEnv;
*/

#define POS_TRANSLATE 0
#define POS_ROTATE 1
#define POS_FOCUS 2

#define STATE_TRIANGLE_OFFSET_COUNT 0
#define STATE_LAST_FOCUS 1


// The script enviroment has 3 env allocations.
// bank0: (r) The enviroment structure
// bank1: (r) The position information
// bank2: (rw) The temporary texture state

int main(int index)
{
    int f1,f2,f3,f4, f5,f6,f7,f8, f9,f10,f11,f12, f13,f14,f15,f16;
    int g1,g2,g3,g4, g5,g6,g7,g8, g9,g10,g11,g12, g13,g14,g15,g16;

    float trans = loadF(1, POS_TRANSLATE);
    float rot = loadF(1, POS_ROTATE);
    matrixLoadScale(&f16, 2.f, 2.f, 2.f);
    matrixTranslate(&f16, 0.f, 0.f, trans);
    matrixRotate(&f16, 90.f, 0.f, 0.f, 1.f);
    matrixRotate(&f16, rot, 1.f, 0.f, 0.f);
    storeMatrix(3, 0, &f16);

    //materialDiffuse(con, 0.0f, 0.0f, 0.0f, 1.0f);
    //materialSpecular(con, 0.5f, 0.5f, 0.5f, 0.5f);
    //materialShininess(intToFloat(20));
    drawTriangleMesh(NAMED_mesh);


    // Start of images.
    bindProgramFragmentStore(NAMED_PFSImages);
    bindProgramFragment(NAMED_PFImages);
    bindProgramVertex(NAMED_PVImages);

    float focusPos = loadF(1, POS_FOCUS);
    int focusID = 0;
    int lastFocusID = loadI32(2, STATE_LAST_FOCUS);
    int imgCount = 13;

    if (trans > (-.3f)) {
        focusID = -1.0f - focusPos;
        if (focusID >= imgCount) {
            focusID = -1;
        }
    } else {
        focusID = -1;
    }

    /*
    if (focusID != lastFocusID) {
        if (lastFocusID >= 0) {
            uploadToTexture(con, env->tex[lastFocusID], 1);
        }
        if (focusID >= 0) {
            uploadToTexture(con, env->tex[focusID], 0);
        }
    }
    */
    storeI32(2, STATE_LAST_FOCUS, focusID);

    int triangleOffsetsCount = loadI32(2, STATE_TRIANGLE_OFFSET_COUNT);

    int imgId = 0;
    for (imgId=1; imgId <= imgCount; imgId++) {
        float pos = focusPos + imgId + 0.4f;
        int offset = (int)floorf(pos * 2.f);
        pos = pos - 0.75f;

        offset = offset + triangleOffsetsCount / 2;

    int drawit = 1;
    if (offset < 0) {
        drawit = 0;
    }
    if (offset >= triangleOffsetsCount) {
        drawit = 0;
    }

        //if (!((offset < 0) || (offset >= triangleOffsetsCount))) {
        if (drawit) {
            int start = offset -2;
            int end = offset + 2;

            if (start < 0) {
                start = 0;
            }
            if (end > triangleOffsetsCount) {
                end = triangleOffsetsCount;
            }

            bindTexture(NAMED_PFImages, 0, loadI32(0, imgId - 1));
            matrixLoadTranslate(&f16, -pos - loadF(5, triangleOffsetsCount / 2), 0, 0);
            vpLoadTextureMatrix(&f16);
            drawTriangleMeshRange(NAMED_mesh, loadI32(4, start), loadI32(4, end) - loadI32(4, start));
        }
    }
    return 0;
}

