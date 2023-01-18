#pragma once
#include <string>
#include <exception>
#include <Windows.h>

void WinApi_Initialize();

void WinApi_Shutdown();

std::string WinApi_ReadAuthToken();

void WinApi_StoreAuthToken(const std::string& token);

class WinApiError : public std::exception
{
public:
    WinApiError(HRESULT errorCode, const std::string& message);

    HRESULT errorCode() const noexcept;

private:

    HRESULT _errorCode;
};