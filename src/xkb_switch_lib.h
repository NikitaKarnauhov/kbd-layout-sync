#pragma once

#include <filesystem>
#include <string>
#include <functional>

class XkbSwitchLib
{
public:
    explicit XkbSwitchLib(std::filesystem::path file_path);
    ~XkbSwitchLib();

    std::string get_layout() const;
    void set_layout(const std::string & layout) const;

private:
    void * const module_handle_;
    std::function<const char *(const char *)> xkb_switch_getxkblayout_;
    std::function<const char *(const char *)> xkb_switch_setxkblayout_;
};
