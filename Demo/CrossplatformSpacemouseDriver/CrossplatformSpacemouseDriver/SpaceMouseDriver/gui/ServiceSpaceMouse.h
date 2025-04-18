#pragma once
#include "BaseThread.h"
#include <vector>

class ServiceSpaceMouse: public BaseThread
{
public:
    ServiceSpaceMouse();
    void run() override;
private:
    std::mutex mutex_arr;
};
