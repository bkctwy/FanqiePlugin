// FanqiePlugin.cpp
#include "FanqiePlugin.h"

extern "C"
{
    IPlugin *createPlugin()
    {
        return new FanqiePlugin();
    }
}