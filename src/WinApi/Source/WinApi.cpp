#include "pch.h"
#include "WinApi.h"

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
    PasswordVault vault;
    auto cred = vault.Retrieve(L"HomeAssistantDesktop", L"Bearer");
    return to_string(cred.Password());
}

void WinApi_StoreAuthToken(const std::string& token)
{
    PasswordCredential cred(L"HomeAssistantDesktop", L"Bearer", to_hstring(token));
    PasswordVault vault;
    vault.Add(cred);
}