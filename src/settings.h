// vi: ts=4 sw=4 tw=100 et

#pragma once

#include <filesystem>
#include <map>
#include <string>

struct Settings
{
    std::string receiver_host;
    std::string receiver_port;
    std::map<std::string, std::string> keyboard_groups;
    std::filesystem::path xkbswitchlib_path;
};

Settings load_settings();
void save_settings(const Settings & settings);
