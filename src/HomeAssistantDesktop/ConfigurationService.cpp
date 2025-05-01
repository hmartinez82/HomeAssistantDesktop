#include "ConfigurationService.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include "WinApi.h"

static const auto REG_VALUE_STARTUP = QStringLiteral("Home Assistant Desktop");

ConfigurationService::ConfigurationService(QObject* parent) : QObject(parent)
{
	WinApi_Shutdown();
	WinApi_Initialize();

	_appPath = '"' % QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) % '"';
}

ConfigurationService::~ConfigurationService()
{
	WinApi_Shutdown();
}

bool ConfigurationService::IsAuthTokenSet() const
{
	return WinApi_IsAuthTokenSet();
}

QString ConfigurationService::GetAuthToken() const
{
	return QString::fromStdString(WinApi_ReadAuthToken());
}

void ConfigurationService::SetAuthToken(const QString& token)
{
    WinApi_StoreAuthToken(token.toStdString());
}

bool ConfigurationService::InputAuthToken()
{
	qDebug() << "Prompting for API token";

	bool ok = false;
	QString token;
	do
	{
		QString token = QInputDialog::getText(nullptr, "Authentication token",
			"Please enter your Home Assistant API token:",
			QLineEdit::Normal,
			QString(),
			&ok);
		if (ok)
		{
			if (token.isEmpty())
			{
				qDebug() << "Empty API token provided. Prompting again";
				QMessageBox::warning(nullptr, "Invalid token", "The provided token is invalid or empty. Try again.", QMessageBox::Ok);
			}
			else
			{
				qInfo() << "Storing new API token token";
				SetAuthToken(token);
				return true;
			}
		}
		else
		{
			return false;
		}
	} while (true);
}

bool ConfigurationService::GetStartWithWindows() const
{
	QSettings settings(R"(HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run)", QSettings::Registry64Format);
	auto var = settings.value(REG_VALUE_STARTUP);
	if (var.isValid())
	{
		return var.toString() == _appPath;
	}
	else
	{
		return false;
	}
	
}

void ConfigurationService::SetStartWithWindows(bool startWithWindows)
{
	QSettings settings(R"(HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run)", QSettings::Registry64Format);
	if (startWithWindows)
	{
		if (settings.value(REG_VALUE_STARTUP) != _appPath)
		{
			settings.setValue(REG_VALUE_STARTUP, _appPath);
		}
	}
	else
	{
		settings.remove(REG_VALUE_STARTUP);
	}
}
