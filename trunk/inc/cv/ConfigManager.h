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

enum ConfigType {
    CONFIG_TYPE_INTEGER,
    CONFIG_TYPE_STRING,
    CONFIG_TYPE_BOOLEAN,
    CONFIG_TYPE_COLOR
};

//-----------------------------------//

// this class stores both the value and the type of
// a ConfigOption; it doesn't store the name because
// they are stored in a map by name
struct ConfigOption
{
    QString value;
    ConfigType type;

    ConfigOption(const QString &v, ConfigType t = CONFIG_TYPE_STRING)
        : value(v),
          type(t)
    { }
};

//-----------------------------------//

class ConfigEvent : public Event
{
    QString     m_filename;
    QString     m_name;
    QString     m_value;
    ConfigType  m_type;

public:
    ConfigEvent(const QString &filename, const QString &name, const QString &value, ConfigType type)
        : m_filename(filename),
          m_name(name),
          m_value(value),
          m_type(type)
    { }

    QString getFilename() const { return m_filename; }
    QString getName() const { return m_name; }
    QString getValue() const { return m_value; }
    ConfigType getType() const { return m_type; }
};

//-----------------------------------//

class ConfigManager
{
    QHash<QString, QMap<QString, ConfigOption> > m_files;
    QString m_defaultFilename;

    // comments start the line with '#'
    QRegExp m_commentRegex;
    QRegExp m_newlineRegex;

public:
    ConfigManager(const QString &defaultFilename);

    bool setupConfigFile(const QString &filename, QMap<QString, ConfigOption> &options);
    bool writeToFile(const QString &filename);
    QString getOptionValue(const QString &filename, const QString &optName);
    bool setOptionValue(const QString &filename, const QString &optName, const QString &optValue, bool fireEvent = false);
    bool isValueValid(const QString &value, ConfigType type);
    void printFile(const QString &filename);

    // calls setupConfigFile() with the default filename
    bool setupDefaultConfigFile(QMap<QString, ConfigOption> &options)
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
