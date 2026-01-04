// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Human.hpp"
#include "Island.hpp"
#include "Json.hpp"
#include <vector>

#define MAX_SAVE_SLOTS 7

struct SaveSlot
{
    int seed = -1;
    std::string name = "Empty slot";
    std::vector<Island> islands;
    std::vector<Human> people;
    int woodTotal = 0, ironTotal = 0, peopleTotal = 0;
    Vector2 mapSize{300, 300};

    Json ToJSON();
    void LoadJSON(Json& json);
};

extern std::vector<SaveSlot> saveSlots;
extern int currentSlot;

void SaveToSlot(int idx);
void LoadFromSlot(int idx);
void EmptySlot(int idx);
void SaveProgress();
void LoadProgress();
