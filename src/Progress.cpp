// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Drawing.hpp"
#include "Human.hpp"
#include "Perlin.hpp"
#include "Progress.hpp"
#include "Settings.hpp"
#include <ctime>
#include <filesystem>

std::vector<SaveSlot> saveSlots(MAX_SAVE_SLOTS);
int currentSlot = -1;

Json SaveSlot::ToJSON()
{
    Json json;

    json["seed"] = seed;
    json["name"] = name;
    for (auto& island: this->islands)
    {
        json["islands"].push_back(island.ToJSON());
    }
    for (auto& human: this->people)
    {
        json["people"].push_back(human.ToJSON());
    }
    json["woodTotal"] = this->woodTotal;
    json["ironTotal"] = this->ironTotal;
    json["peopleTotal"] = this->peopleTotal;

    json["mapSize"].format = JsonFormat::Inline;
    json["mapSize"].push_back(this->mapSize.x);
    json["mapSize"].push_back(this->mapSize.y);

    return json;
}

void SaveSlot::LoadJSON(Json& json)
{
    seed = json["seed"].GetDouble();
    name = json["name"].GetString();
    this->islands.clear();
    for (size_t i = 0; i < json["islands"].size(); i++)
    {
        this->islands.push_back(Island::LoadJSON(json["islands"][i]));
        this->islands.back().index = i;
    }
    this->people.clear();
    for (size_t i = 0; i < json["people"].size(); i++)
    {
        this->people.push_back(Human::LoadJSON(json["people"][i]));
    }
    this->woodTotal = json["woodTotal"].GetInt();
    this->ironTotal = json["ironTotal"].GetInt();
    this->peopleTotal = json["peopleTotal"].GetInt();
    this->mapSize = {static_cast<float>(json["mapSize"][0].GetDouble()),
                     static_cast<float>(json["mapSize"][1].GetDouble())};
}

void SaveToSlot(int idx)
{
    if (idx < 0) return;
    saveSlots[idx].seed = perlinSeed;
    saveSlots[idx].islands = islands;
    saveSlots[idx].people = people;
    saveSlots[idx].woodTotal = woodTotal;
    saveSlots[idx].ironTotal = ironTotal;
    saveSlots[idx].peopleTotal = peopleTotal;
    saveSlots[idx].name = labels["Slot"] + " " + std::to_string(idx + 1);
    saveSlots[idx].mapSize = mapSize;
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
    people = saveSlots[idx].people;
    woodTotal = saveSlots[idx].woodTotal;
    ironTotal = saveSlots[idx].ironTotal;
    peopleTotal = saveSlots[idx].peopleTotal;
    mapSize = saveSlots[idx].mapSize;

    SetShaderValue(biomeShader, GetShaderLocation(biomeShader, "uSeed"), &perlinSeed,
                   SHADER_UNIFORM_INT);
}

void EmptySlot(int idx) { saveSlots[idx] = {}; }

void SaveProgress()
{
    SaveToSlot(currentSlot);

    Json json;

    json["version"] = 2;

    for (size_t i = 0; i < MAX_SAVE_SLOTS; i++)
    {
        json["saves"].push_back(saveSlots[i].ToJSON());
    }

    json.Save("saves.json");
}

void MigrateV0()
{
    int lastPerlinSeed = perlinSeed;
    for (auto& slot: saveSlots)
    {
        perlinSeed = slot.seed;
        for (size_t i = 0; i < slot.islands.size(); i++)
        {
            auto& island = slot.islands[i];
            if (!island.colonized) continue;
            for (int j = 0; j < island.peopleCount; j++)
            {
                slot.people.emplace_back(island.GetRandomPoint(), i);
            }
        }
    }
    perlinSeed = lastPerlinSeed;
}

void MigrateV1()
{
    for (auto& slot: saveSlots)
    {
        slot.mapSize = {300, 300};
    }
}

void LoadProgress()
{
    if (!std::filesystem::exists("saves.json"))
    {
        SaveProgress();
        return;
    }

    Json json = Json::Load("saves.json");

    int version = json["version"].GetInt();

    for (size_t i = 0; i < json["saves"].size(); i++)
    {
        saveSlots[i].LoadJSON(json["saves"][i]);
    }

    if (version == 0)
    {
        MigrateV0();
        version = 1;
    }
    if (version == 1)
    {
        MigrateV1();
        version = 2;
    }
}
