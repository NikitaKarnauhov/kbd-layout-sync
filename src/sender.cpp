#include "sender.h"

#include <cstdlib>
#include <stdexcept>

#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace
{

void send_impl(const int fd, const char * const buf, const std::size_t size) {
    for (int retry = 5; retry >= 0; --retry) {
        const auto sent_size = ::send(fd, buf, size, 0);

        if (sent_size == size)
        {
            break;
        }

        if (sent_size < 0 && (errno != ECONNREFUSED || retry == 0))
        {
            throw std::runtime_error("send()");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

}

Sender::Sender(
    const std::string & host,
    const std::string & port,
    const std::map<std::string, std::string> & keyboard_groups)
    : host_{host}
    , port_{port}
    , keyboard_groups_{keyboard_groups}
{
}

void Sender::run()
{
    Display * const display = XOpenDisplay(NULL);

    if (display == nullptr)
    {
        throw std::runtime_error("XOpenDisplay()");
    }

    const int fd = ::socket(PF_INET, SOCK_DGRAM, 0);

    if (fd < 0)
    {
        throw std::runtime_error("socket()");
    }

    struct hostent * const host = ::gethostbyname(host_.c_str());

    if (!host)
    {
        throw std::runtime_error("gethostbyname()");
    }

    struct sockaddr_in name;

    name.sin_family = AF_INET;
    name.sin_port = ::htons(std::strtoul(port_.c_str(), nullptr, 10));
    name.sin_addr = *(struct in_addr *)host->h_addr;

    if (::connect(fd, (struct sockaddr *)&name, sizeof(name)) < 0)
    {
        throw std::runtime_error("connect()");
    }

    int xkb_event_type;

    XkbQueryExtension(display, 0, &xkb_event_type, 0, 0, 0);
    XkbSelectEvents(display, XkbUseCoreKbd, XkbAllEventsMask, XkbAllEventsMask);
    XSync(display, False);

    int last_lang = -1;

    const int listen_fd = ConnectionNumber(display);
    struct pollfd poll_fd{listen_fd, POLLIN, 0};

    while (true)
    {
        if (::poll(&poll_fd, 1, 1000) < 0)
        {
            throw std::runtime_error("poll()");
        }

        if (should_stop())
        {
            break;
        }

        if ((poll_fd.revents & POLLIN) == 0)
        {
            continue;
        }

        while (XPending(display))
        {
            XEvent event;
            XNextEvent(display, &event);
            if (event.type == xkb_event_type)
            {
                const XkbEvent * const xkb_event = reinterpret_cast<const XkbEvent *>(&event);

                if (xkb_event->any.xkb_type == XkbStateNotify)
                {
                    const int lang = xkb_event->state.group;

                    if (lang == last_lang)
                    {
                        continue;
                    }

                    last_lang = lang;

                    const auto group_it = keyboard_groups_.find(std::to_string(lang));

                    if (group_it != keyboard_groups_.cend())
                    {
                        send_impl(fd, group_it->second.data(), group_it->second.size());
                    }
                }
            }
        }
    }

    ::close(fd);
}
