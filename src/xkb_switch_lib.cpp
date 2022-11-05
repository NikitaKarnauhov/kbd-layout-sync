#include "xkb_switch_lib.h"

#include <dlfcn.h>

using namespace std::string_literals;

namespace
{

template <typename T>
inline T * lookup_symbol(void * const mod, const char * const name)
{
    return reinterpret_cast<T *>(::dlsym(mod, name));
}

}

XkbSwitchLib::XkbSwitchLib(std::filesystem::path file_path)
    : module_handle_{::dlopen(file_path.string().c_str(), RTLD_NOW | RTLD_LOCAL)}
{
    if (module_handle_ == nullptr)
    {
        return;
        // throw std::runtime_error("dlopen(): "s + ::dlerror());
    }

    xkb_switch_getxkblayout_ = lookup_symbol<const char *(const char *)>(module_handle_, "Xkb_Switch_getXkbLayout");
    xkb_switch_setxkblayout_ = lookup_symbol<const char *(const char *)>(module_handle_, "Xkb_Switch_setXkbLayout");
}

XkbSwitchLib::~XkbSwitchLib()
{
    if (module_handle_ != nullptr)
    {
        ::dlclose(module_handle_);
    }
}

std::string XkbSwitchLib::get_layout() const
{
    return xkb_switch_getxkblayout_ ? std::string{xkb_switch_getxkblayout_("")} : ""s;
}

void XkbSwitchLib::set_layout(const std::string & layout) const
{
    if (xkb_switch_setxkblayout_)
    {
        xkb_switch_setxkblayout_(layout.c_str());
    }
}
