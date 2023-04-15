#ifndef _HTTPHANDLER_H_
#define _HTTPHANDLER_H_

#include "eventhandler.h"

class HTTPHandler : public EventHandler
{
public:
    void handleEvent(int fd, uint32_t events) override;

    ~HTTPHandler();
};

#endif // _HTTPHANDLER_H_