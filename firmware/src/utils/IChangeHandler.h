#ifndef ICHANGEHANDLER_H
#define ICHANGEHANDLER_H
#include <Arduino.h>

class IChangeHandler
{
public:
    virtual void handleChangeEvent() = 0;
    virtual void handleCalibrationEvent() = 0;
};


#endif // !ICHANGEHANDLER_H