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
    QString value;

public:
    ConfigOption(const QString &n, const QString &v)
     : name(n),
       value(v)
    { }

    void setName(const QString &n) { name = n; }
    QString getName() const { return name; }
    void setValue(const QString &v) { value = v; }
    QString getValue() const { return value; }
};
