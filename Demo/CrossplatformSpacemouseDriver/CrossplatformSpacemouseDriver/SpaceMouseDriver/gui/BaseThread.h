#pragma once

#include <atomic>
#include <string>
#include <thread>
#include <mutex>
class BaseThread
{
public:
    BaseThread();
    virtual ~BaseThread();
    virtual void Start();
    void Stop();
    bool isRuning();
    void msleep(const long long& dely);
    std::string get_error_str();
protected:
    virtual void run();
    std::string name;
    std::string error_str;
private:
    std::atomic<bool> isRun;
    std::thread thread;
};
