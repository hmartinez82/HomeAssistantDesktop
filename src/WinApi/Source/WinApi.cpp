#include "pch.h"
#include "WinApi.h"
#include <system_error>
#include <algorithm>
#include <iterator>

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

bool WinApi_IsAuthTokenSet()
{
    using namespace std;
    try
    {
        PasswordVault vault;
		auto creds = vault.FindAllByResource(L"HomeAssistantDesktop");
        
        // Check if any credentials exist for the resource
        auto it = std::find_if(cbegin(creds), cend(creds), [](const PasswordCredential& cred) {
            return cred.UserName() == L"Bearer";
        });

		// Check if the password (token) is not empty for the found credential
		if (it != cend(creds))
		{
            auto cred = vault.Retrieve(L"HomeAssistantDesktop", (*it).UserName());
			return !cred.Password().empty();
		}

        return false;
    }
    catch (winrt::hresult_error const& ex)
    {
        throw WinApiError(ex.code().value, to_string(ex.message()));
    }
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
