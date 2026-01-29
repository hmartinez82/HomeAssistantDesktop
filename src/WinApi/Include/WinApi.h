#pragma once
#include <string>
#include <exception>
#include <Windows.h>

void WinApi_Initialize();

void WinApi_Shutdown();

bool WinApi_IsAuthTokenSet();

std::string WinApi_ReadAuthToken();

void WinApi_StoreAuthToken(const std::string& token);

bool WinApi_GetStartupEnabled();

void WinApi_SetStartupEnabled(bool enable);

class WinApiError : public std::exception
{
public:
    WinApiError(HRESULT errorCode, const std::string& message);

    HRESULT errorCode() const noexcept;

private:

    HRESULT _errorCode;
};