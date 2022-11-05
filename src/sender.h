#pragma once

#include "worker.h"

#include <string>
#include <map>
#include <thread>
#include <atomic>

class Sender : public Worker
{
public:
    Sender(
        const std::string & host,
        const std::string & port,
        const std::map<std::string, std::string> & keyboard_groups);

protected:
    void run() override;

private:
    const std::string host_;
    const std::string port_;
    const std::map<std::string, std::string> keyboard_groups_;
};
