#ifndef _EVENTHANDLER_H_
#define _EVENTHANDLER_H_

#include <stdint.h>
#include "threadpool.h"

class EventHandler // 创建子类实现虚函数 子类可以是处理TCP的也可以是处理UDP的
{
public:
    virtual void handleEvent(int &fd, uint32_t &events) = 0; // 回调函数 需要创建子类实现

    virtual ~EventHandler();
};

#endif // _EVENTHANDLER_H_