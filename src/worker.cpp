#include "worker.h"

#include <cassert>
#include <stdexcept>
#include <iostream>

Worker::~Worker()
{
    assert(status_ != Status::Running);
    if (thread_.joinable())
    {
        thread_.join();
    }
}

void Worker::start()
{
    Status stopped{Status::Stopped};
    if (status_.compare_exchange_weak(stopped, Status::Running))
    {
        if (thread_.joinable())
        {
            thread_.join();
        }

        thread_ = std::thread{[this] {
            try
            {
                run();
            }
            catch (const std::exception & e)
            {
                std::cerr << "Error: " << e.what() << std::endl;
                status_ = Status::Stopped;
            }
        }};
    }
}

void Worker::stop()
{
    Status running{Status::Running};
    if (status_.compare_exchange_weak(running, Status::Stopping))
    {
        thread_.join();
    }
}

Status Worker::status() const
{
    return status_;
}

bool Worker::should_stop()
{
    Status stopping{Status::Stopping};
    return status_.compare_exchange_weak(stopping, Status::Stopped);
}
