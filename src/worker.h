#pragma once

#include "status.h"

#include <thread>
#include <atomic>

class Worker
{
public:
    virtual ~Worker();

    virtual void start();
    virtual void stop();
    virtual Status status() const;

protected:
    virtual void run() = 0;
    virtual bool should_stop();

protected:
    std::thread thread_;
    std::atomic<Status> status_{Status::Stopped};
};
