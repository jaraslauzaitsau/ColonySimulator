// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstdlib>

inline float GetRandomFloat(float a, float b) { return rand() * 1.0f / RAND_MAX * (b - a) + a; }
