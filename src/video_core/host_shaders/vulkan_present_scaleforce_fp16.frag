// SPDX-FileCopyrightText: 2021 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#version 460
#extension GL_GOOGLE_include_directive : enable

#ifndef VERSION
#define VERSION 2
#endif

#ifndef YUZU_USE_FP16
#define YUZU_USE_FP16
#endif

#include "opengl_present_scaleforce.frag"

