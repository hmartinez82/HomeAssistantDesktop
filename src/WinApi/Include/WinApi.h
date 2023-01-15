#pragma once
#include <string>

void WinApi_Initialize();


void WinApi_Shutdown();

std::string WinApi_ReadAuthToken();

void WinApi_StoreAuthToken(const std::string& token);