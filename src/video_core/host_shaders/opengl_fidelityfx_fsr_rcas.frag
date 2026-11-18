// SPDX-FileCopyrightText: Copyright 2023 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#version 460 core
#extension GL_GOOGLE_include_directive : enable

#ifndef USE_RCAS
#define USE_RCAS 1
#endif

#include "opengl_fidelityfx_fsr.frag"
