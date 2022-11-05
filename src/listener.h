#pragma once

#include "worker.h"

#include <functional>
#include <string>

using OnLayoutReceived = std::function<void(std::string)>;

class Listener : public Worker
{
public:
    Listener(
        const std::string & host,
        const std::string & port,
        OnLayoutReceived on_layout_received);

protected:
    void run() override;

private:
    const std::string host_;
    const std::string port_;
    const OnLayoutReceived on_layout_received_;
};
