#include "pch.h"
#include "WinApi.h"

void WinApi_Initialize()
{
    winrt::init_apartment();
}


void WinApi_Shutdown()
{
    winrt::uninit_apartment();
}
