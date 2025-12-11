// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Settings.hpp"
#include <fstream>
#include <vector>

bool vsync = true;
bool showFPS = true;
double perlinScale = 0.01;
double panSensitivity = 500;
double wheelSensitivity = 0.1;

std::vector<std::string> Split(std::string input, char delimiter = ' ')
{
    std::vector<std::string> output;
    output.push_back("");
    int index = 0;
    for (size_t i = 0; i < input.size(); i++)
    {
        if (input[i] == delimiter)
        {
            index++;
            output.push_back("");
            continue;
        }
        output[index] += input[i];
    }
    return output;
}

void Save()
{
    std::ofstream file("settings.txt");
    file << "vsync=" << (vsync ? "true" : "false") << '\n';
    file << "show-fps=" << (showFPS ? "true" : "false") << '\n';
    file << "perlin-scale=" << perlinScale << '\n';
    file.close();
}

void Load()
{
    std::ifstream file("settings.txt");
    std::string buf, label, value;
    while (std::getline(file, buf))
    {
        label = Split(buf, '=')[0];
        value = Split(buf, '=')[1];
        if (label == "vsync") vsync = value == "true";
        if (label == "show-fps") showFPS = value == "true";
        if (label == "perlin-scale") perlinScale = stod(value);
    }
    file.close();
}
