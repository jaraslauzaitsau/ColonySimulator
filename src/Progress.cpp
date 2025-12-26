// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Progress.hpp"

#include "Drawing.hpp"
#include "Perlin.hpp"
#include <ctime>
#include <filesystem>

std::vector<SaveSlot> saveSlots(MAX_SAVE_SLOTS);
int currentSlot = -1;

JSON SaveSlot::ToJSON()
{
    JSON json;

    json["seed"] = seed;
    json["name"] = name;
    for (auto& island: this->islands)
    {
        json["islands"].push_back(island.ToJSON());
    }
    json["woodTotal"] = this->woodTotal;
    json["ironTotal"] = this->ironTotal;
    json["peopleTotal"] = this->peopleTotal;

    return json;
}

void SaveSlot::LoadJSON(JSON& json)
{
    seed = json["seed"].GetDouble();
    name = json["name"].GetString();
    this->islands.clear();
    for (size_t i = 0; i < json["islands"].size(); i++)
    {
        this->islands.push_back(Island::LoadJSON(json["islands"][i]));
    }
    this->woodTotal = json["woodTotal"].GetInt();
    this->ironTotal = json["ironTotal"].GetInt();
    this->peopleTotal = json["peopleTotal"].GetInt();
}

void SaveToSlot(int idx)
{
    if (idx < 0) return;
    saveSlots[idx].seed = perlinSeed;
    saveSlots[idx].islands = islands;
    saveSlots[idx].woodTotal = woodTotal;
    saveSlots[idx].ironTotal = ironTotal;
    saveSlots[idx].peopleTotal = peopleTotal;
    saveSlots[idx].name = "Slot " + std::to_string(idx + 1);
}

void LoadFromSlot(int idx)
{
    currentSlot = idx;
    if (saveSlots[idx].seed == -1)
    {
        BuildMap();
        SaveToSlot(idx);
        return;
    }

    perlinSeed = saveSlots[idx].seed;
    islands = saveSlots[idx].islands;
    woodTotal = saveSlots[idx].woodTotal;
    ironTotal = saveSlots[idx].ironTotal;
    peopleTotal = saveSlots[idx].peopleTotal;

    SetShaderValue(biomeShader, GetShaderLocation(biomeShader, "uSeed"), &perlinSeed,
                   SHADER_UNIFORM_INT);
}

void EmptySlot(int idx) { saveSlots[idx] = {}; }

void SaveProgress()
{
    SaveToSlot(currentSlot);

    JSON json;

    json["version"] = 0;

    for (size_t i = 0; i < MAX_SAVE_SLOTS; i++)
    {
        json["saves"].push_back(saveSlots[i].ToJSON());
    }

    json.Save("saves.json");
}

void LoadProgress()
{
    if (!std::filesystem::exists("saves.json"))
    {
        SaveProgress();
        return;
    }

    JSON json = JSON::Load("saves.json");

    // The version isn't needed yet
    // int version = json["version"].GetInt();

    for (size_t i = 0; i < json["saves"].size(); i++)
    {
        saveSlots[i].LoadJSON(json["saves"][i]);
    }
}
