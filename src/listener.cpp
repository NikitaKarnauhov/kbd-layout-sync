#include "listener.h"

#include <cassert>
#include <regex>
#include <stdexcept>
#include <vector>

#include <QScopeGuard>

#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Listener::Listener(
    const std::string & host,
    const std::string & port,
    OnLayoutReceived on_layout_received)
    : host_{host}
    , port_{port}
    , on_layout_received_{std::move(on_layout_received)}
{
    assert(on_layout_received_ != nullptr);
}

void Listener::run()
{
    struct addrinfo hints = {};
    struct addrinfo * server_info = nullptr;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if (::getaddrinfo(host_.c_str(), port_.c_str(), &hints, &server_info) != 0)
    {
        throw std::runtime_error("getaddrinfo()");
    }

    const int listen_fd = ::socket(
        server_info->ai_family,
        server_info->ai_socktype,
        server_info->ai_protocol);

    const auto guard = qScopeGuard([&]
    {
        ::close(listen_fd);
    });

    int sockopt = 1;
    if (::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt)) != 0)
    {
        throw std::runtime_error("setsockopt()");
    }

    if (::bind(listen_fd, server_info->ai_addr, server_info->ai_addrlen) != 0)
    {
        throw std::runtime_error("bind()");
    }

    ::freeaddrinfo(server_info);

    struct pollfd poll_fd{listen_fd, POLLIN, 0};
    constexpr std::size_t buffer_size = 1024;
    std::vector<char> buffer(buffer_size);

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

        struct sockaddr sender = {0};
        socklen_t sender_size = sizeof(sender);

        const int packet_size = ::recvfrom(
            poll_fd.fd,
            buffer.data(),
            buffer.size(),
            0,
            &sender,
            &sender_size);

        if (packet_size < 0)
        {
            throw std::runtime_error("recvfrom()");
        }

        if (on_layout_received_)
        {
            static std::regex word{R"(\w+)"};
            std::string data_str(buffer.data(), buffer.size());
            std::smatch result;
            if (std::regex_search(data_str, result, word))
            {
                on_layout_received_(result.str(0));
            }
        }
    }
}

