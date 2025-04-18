#include "BaseThread.h"
#include <chrono>

BaseThread::BaseThread(): name(typeid(decltype(*this)).name()),isRun(false)
{
}

BaseThread::~BaseThread()
{
    this->Stop();
}

void BaseThread::Start()
{
    this->isRun.store(true);
    this->thread = std::thread(&BaseThread::run, this);
    this->thread.detach();
}

void BaseThread::run()
{

}

void BaseThread::Stop()
{
    this->isRun.store(false);
}

std::string BaseThread::get_error_str()
{
    return error_str;
}

void BaseThread::msleep(const long long& dely)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(dely));
}

bool BaseThread::isRuning()
{
    return this->isRun.load();
}
