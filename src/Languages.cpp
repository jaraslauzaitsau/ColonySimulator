// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Languages.hpp"
#include "Progress.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

std::string currentLanguage = "en";
std::vector<std::string> languages;
std::unordered_map<std::string, std::string> labels;

void GetAllLanguages()
{
    languages.clear();
    for (auto& file: std::filesystem::directory_iterator("resources/languages"))
    {
        if (!file.is_regular_file() || file.path().extension() != ".txt") continue;
        languages.push_back(file.path().stem().string());
    }
}

void ReloadLabels()
{
    labels.clear();
    std::ifstream englishFile("resources/languages/en.txt"),
        targetFile("resources/languages/" + currentLanguage + ".txt");
    if (!englishFile || !targetFile) return;
    std::string enBuf, targetBuf;
    while (std::getline(englishFile, enBuf) && std::getline(targetFile, targetBuf))
    {
        labels[enBuf] = targetBuf;
    }

    for (auto& slot: saveSlots)
    {
        if (slot.seed == -1) slot.name = labels["Empty slot"];
    }
}
