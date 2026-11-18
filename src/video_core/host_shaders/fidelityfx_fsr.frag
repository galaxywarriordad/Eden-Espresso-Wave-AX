// SPDX-FileCopyrightText: Copyright 2021 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#define A_GPU 1
#define A_GLSL 1
#ifdef YUZU_USE_FP16
    #define A_HALF 1
#endif
#include "ffx_a.h"

#ifndef FSR_CONSTANTS_BOUND
#define FSR_CONSTANTS_BOUND
layout(push_constant) uniform constants {
    uvec4 Const0;
    uvec4 Const1;
    uvec4 Const2;
    uvec4 Const3;
};

layout(set = 0, binding = 0) uniform sampler2D InputTexture;
#endif

#if defined(USE_EASU) && USE_EASU
    #ifndef FSR_EASU_F
        #define FSR_EASU_F 1
    #endif
    AF4 FsrEasuRF(AF2 p) { return textureGather(InputTexture, p, 0); }
    AF4 FsrEasuGF(AF2 p) { return textureGather(InputTexture, p, 1); }
    AF4 FsrEasuBF(AF2 p) { return textureGather(InputTexture, p, 2); }
    #ifdef YUZU_USE_FP16
        #ifndef FSR_EASU_H
            #define FSR_EASU_H 1
        #endif
        AH4 FsrEasuRH(AF2 p) { return AH4(textureGather(InputTexture, p, 0)); }
        AH4 FsrEasuGH(AF2 p) { return AH4(textureGather(InputTexture, p, 1)); }
        AH4 FsrEasuBH(AF2 p) { return AH4(textureGather(InputTexture, p, 2)); }
    #endif
#endif

#if defined(USE_RCAS) && USE_RCAS
    #define FSR_RCAS_PASSTHROUGH_ALPHA 1
    #ifndef FSR_RCAS_F
        #define FSR_RCAS_F 1
    #endif
    AF4 FsrRcasLoadF(ASU2 p) { return texelFetch(InputTexture, ASU2(p), 0); }
    void FsrRcasInputF(inout AF1 r, inout AF1 g, inout AF1 b) {}
    #ifdef YUZU_USE_FP16
        #ifndef FSR_RCAS_H
            #define FSR_RCAS_H 1
        #endif
        AH4 FsrRcasLoadH(ASW2 p) { return AH4(texelFetch(InputTexture, ASU2(p), 0)); }
        void FsrRcasInputH(inout AH1 r, inout AH1 g, inout AH1 b) {}
    #endif
#endif

#include "ffx_fsr1.h"

layout(location = 0) in vec2 frag_texcoord;
layout(location = 0) out vec4 frag_color;

void CurrFilter(AU2 pos) {
#if defined(USE_EASU) && USE_EASU
    #ifndef YUZU_USE_FP16
        AF3 c;
        FsrEasuF(c, pos, Const0, Const1, Const2, Const3);
        frag_color = AF4(c, texture(InputTexture, frag_texcoord).a);
    #else
        AH3 c;
        FsrEasuH(c, pos, Const0, Const1, Const2, Const3);
        frag_color = AH4(c, texture(InputTexture, frag_texcoord).a);
    #endif
#endif

#if defined(USE_RCAS) && USE_RCAS
    #ifndef YUZU_USE_FP16
        AF4 c;
        FsrRcasF(c.r, c.g, c.b, c.a, pos, Const0);
        frag_color = c;
    #else
        AH4 c;
        FsrRcasH(c.r, c.g, c.b, c.a, pos, Const0);
        frag_color = c;
    #endif
#endif
}

void main() {
#if defined(USE_RCAS) && USE_RCAS
    CurrFilter(AU2(frag_texcoord * vec2(textureSize(InputTexture, 0))));
#else
    CurrFilter(AU2(gl_FragCoord.xy));
#endif
}
