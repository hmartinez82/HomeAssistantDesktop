#pragma once

#include <QObject>
#include <QString>

class ConfigurationService : public QObject
{
	Q_OBJECT
public:
	ConfigurationService(QObject *parent = nullptr);

	~ConfigurationService();

	bool IsAuthTokenSet() const;

	QString GetAuthToken() const;

	void SetAuthToken(const QString& token);

	bool InputAuthToken();

	bool GetStartWithWindows() const;

	void SetStartWithWindows(bool startWithWindows);

private:
	QString _appPath;
};

