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

namespace cv {

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
    ConfigManager(const QString &defaultFilename)
      : m_commentRegex("^\\s*#"),
        m_defaultFilename(defaultFilename)
    { }

    bool setupConfigFile(const QString &filename, const QList<ConfigOption> &options);
    bool writeToFile(const QString &filename);
    QString getOptionValue(const QString &filename, const QString &optName);
    bool setOptionValue(const QString &filename, const QString &optName, const QString &optValue);
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
    inline bool setOptionValue(const QString &optName, const QString &optValue)
    {
        return setOptionValue(m_defaultFilename, optName, optValue);
    }
};

extern ConfigManager *g_pCfgManager;

} // end namespace
