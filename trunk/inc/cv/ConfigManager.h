/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QHash>
#include <QString>
#include <QMap>
#include <QList>
#include <QRegExp>
#include "cv/EventManager.h"

#define GET_OPT g_pCfgManager->getOptionValue

namespace cv {

class ConfigEvent : public Event
{
    QString m_filename;
    QString m_optName;
    QString m_optValue;

public:
    ConfigEvent(const QString &filename, const QString &optName, const QString &optValue)
        : m_filename(filename),
          m_optName(optName),
          m_optValue(optValue)
    { }

    QString getFilename() const { return m_filename; }
    QString getName() const { return m_optName; }
    QString getValue() const { return m_optValue; }
};

//-----------------------------------//

struct ConfigOption
{
    QString name;
    QString value;

    ConfigOption(const QString &n, const QString &v)
        : name(n),
          value(v)
    { }
};

//-----------------------------------//

class ConfigManager
{
    QHash<QString, QMap<QString, QString> > m_files;

    // comments start the line with '#'
    QRegExp m_commentRegex;

    QString m_defaultFilename;

public:
    ConfigManager(const QString &defaultFilename);

    bool setupConfigFile(const QString &filename, const QList<ConfigOption> &options);
    bool writeToFile(const QString &filename);
    QString getOptionValue(const QString &filename, const QString &optName);
    bool setOptionValue(const QString &filename, const QString &optName, const QString &optValue, bool fireEvent = false);
    void printFile(const QString &filename);

    // calls setupConfigFile() with the default filename
    bool setupDefaultConfigFile(const QList<ConfigOption> &options)
    {
        return setupConfigFile(m_defaultFilename, options);
    }

    // calls writeToFile with the default filename
    bool writeToDefaultFile()
    {
        return writeToFile(m_defaultFilename);
    }

    // fetches the value of the key in the default file
    inline QString getOptionValue(const QString &optName)
    {
        return getOptionValue(m_defaultFilename, optName);
    }

    // sets the option's value to optValue in the default file
    inline bool setOptionValue(const QString &optName, const QString &optValue, bool fireEvent = false)
    {
        return setOptionValue(m_defaultFilename, optName, optValue, fireEvent);
    }
};

extern ConfigManager *g_pCfgManager;

} // end namespace
