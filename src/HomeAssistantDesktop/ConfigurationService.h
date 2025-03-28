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
};

