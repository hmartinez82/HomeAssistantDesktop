#include "pch.h"
#include "WinApi.h"
#include <system_error>

using namespace winrt;
using namespace Windows::Security::Credentials;

void WinApi_Initialize()
{
    winrt::init_apartment();
}


void WinApi_Shutdown()
{
    winrt::uninit_apartment();
}

std::string WinApi_ReadAuthToken()
{
    try
    {
        PasswordVault vault;
        auto cred = vault.Retrieve(L"HomeAssistantDesktop", L"Bearer");
        return to_string(cred.Password());
    }
    catch (winrt::hresult_error const& ex)
    {
        throw WinApiError(ex.code().value, to_string(ex.message()));
    }
}

void WinApi_StoreAuthToken(const std::string& token)
{
    try
    {
        PasswordCredential cred(L"HomeAssistantDesktop", L"Bearer", to_hstring(token));
        PasswordVault vault;
        vault.Add(cred);
    }
    catch (winrt::hresult_error const& ex)
    {
        throw WinApiError(ex.code().value, to_string(ex.message()));
    }
}

WinApiError::WinApiError(HRESULT errorCode, const std::string& message) : std::exception(message.c_str()),
  _errorCode(errorCode)
{

}

HRESULT WinApiError::errorCode() const noexcept
{
    return  _errorCode;
}
