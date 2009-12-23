/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QString>

class ConfigOption
{
	QString name;
    QString	value;
	
public:
	ConfigOption(const QString &n, const QString &v)
		: name(n),
          value(v)
    { }

	void SetName(const QString &n) { name = n; }
	QString GetName() const { return name; }
	void SetValue(const QString &v) { value = v; }
	QString GetValue() const { return value; }
};
