/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QFile>
#include <QTemporaryFile>
#include <QVariant>
#include <QColor>
#include <QDebug>
#include "cv/ConfigManager.h"
#include "json.h"

namespace cv {

ConfigManager::ConfigManager(const QString &defaultFilename)
    : m_commentRegex("^\\s*#"),
      m_newlineRegex("[\r\n]*"),
      m_defaultFilename(defaultFilename)
{
    g_pEvtManager->createEvent("configChanged", STRING_EVENT);
}

// this function puts options (and their values) into memory under
// a specific filename
//
// if the file with the given name exists, it overwrites any of the
// provided default options with the values from the file
//  - options within the file that are not found in the list of
//      default options are ignored; this allows options to be
//      easily deprecated without replacing the file
//
// if the file does not exist, it also creates a new file with
// the given array of default ConfigOptions
bool ConfigManager::setupConfigFile(const QString &filename, QMap<QString, ConfigOption> &options)
{
    QFile file(filename);
    if(file.exists() && !file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug("[CM::setupConfigFile] File %s could not be opened for read", filename.toLatin1().constData());
        return false;
    }

    if(file.exists())
    {
        // read in the entire file, parse the JSON
        QString jsonStr = file.readAll();
        bool success;
        QVariantMap cfgMap = QtJson::Json::parse(jsonStr, success).toMap();
        if(success)
        {
            // for each default option
            for(QMap<QString, ConfigOption>::iterator i = options.begin(); i != options.end(); ++i)
            {
                // find the option with the same key in the cfg from the file;
                // if it exists AND its value is valid, replace it
                QVariantMap::iterator option = cfgMap.find(i.key());
                if(option != cfgMap.end() && isValueValid(option.value(), i->type))
                    options.insert(i.key(), option.value());
            }
        }
    }
    else
    {
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug("[CM::setupConfigFile] File %s could not be opened for write", filename.toLatin1().constData());
            return false;
        }

        // write all the default options to the file
        QVariantMap cfgMap;
        for(QMap<QString, ConfigOption>::iterator i = options.begin(); i != options.end(); ++i)
            cfgMap.insert(i.key(), i->value);

        file.write(QtJson::Json::serialize(cfgMap));
    }

    file.close();
    m_files.insert(filename, options);
    return true;
}

//-----------------------------------//

// this writes all the data in memory to the provided file
//
// if the file already exists, then for each line it tries to find
// the corresponding option in memory
//  - if the option is found, then it writes the value in memory
//  - if the option is unknown, then it just writes the line from the file
//
// finally, it writes any options that aren't in the file but are
// existing in memory
//  - this allows for adding new options to a file by modifying the default
//      options that are put into memory using SetupConfigFile()
bool ConfigManager::writeToFile(const QString &filename)
{
    QHash<QString, QMap<QString, ConfigOption> >::iterator i = m_files.find(filename);

    if(i != m_files.end())
    {
        QFile file(filename);
        if(file.exists() && !file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            qDebug("[CM::writeToFile] File %s could not be opened for write", filename.toLatin1().constData());
            return false;
        }

        // write all the current options in memory to the file
        QMap<QString, ConfigOption> options = i.value();
        QVariantMap cfgMap;
        for(QMap<QString, ConfigOption>::iterator j = options.begin(); j != options.end(); ++j)
            cfgMap.insert(j.key(), j->value);

        file.write(QtJson::Json::serialize(cfgMap));
        return true;
    }
    else
        qDebug("[CM::writeToFile] Config file %s not found in memory", filename.toLatin1().constData());

    return false;
}

//-----------------------------------//

// returns the value of the provided option inside the provided file,
// returns an empty string upon error
QVariant ConfigManager::getOptionValue(const QString &filename, const QString &optName)
{
    QHash<QString, QMap<QString, ConfigOption> >::iterator i = m_files.find(filename);
    if(i != m_files.end())
    {
        QMap<QString, ConfigOption>::iterator j = i.value().find(optName);
        if(j != i.value().end())
            return j->value;
        else
            qDebug("[CM::getOptionValue] Config option %s (in file %s) does not exist",
                   optName.toLatin1().constData(),
                   filename.toLatin1().constData());
    }
    else
        qDebug("[CM::getOptionValue] Config file %s not found", filename.toLatin1().constData());

    return "";
}

//-----------------------------------//

// sets the provided option's value to optValue
bool ConfigManager::setOptionValue(
        const QString &filename,
        const QString &optName,
        const QVariant &optValue,
        bool fireEvent)
{
    QHash<QString, QMap<QString, ConfigOption> >::iterator i = m_files.find(filename);
    if(i != m_files.end())
    {
        QMap<QString, ConfigOption>::iterator j = i.value().find(optName);
        if(j != i.value().end())
        {
            // if the new value is valid
            if(isValueValid(optValue, j->type))
            {
                // then set the new value and fire the "configChanged" event
                j->value = optValue;
                if(fireEvent)
                {
                    ConfigEvent *pEvent = new ConfigEvent(filename, optName, j->value, j->type);
                    g_pEvtManager->fireEvent("configChanged", optName, pEvent);
                    delete pEvent;
                }
                return true;
            }
            else
                qDebug("[CM::setOptionValue] Value is not valid for config option %s",
                       optName.toLatin1().constData());
        }
        else
            qDebug("[CM::setOptionValue] Config option %s (in file %s) does not exist",
                   optName.toLatin1().constData(),
                   filename.toLatin1().constData());
    }
    else
        qDebug("[CM::setOptionValue] Config file %s not found", filename.toLatin1().constData());

    return false;
}

//-----------------------------------//

// returns true if the [value] can be converted to the given [type],
// returns false otherwise
bool ConfigManager::isValueValid(const QVariant &value, ConfigType type)
{
    bool ok;
    switch(type)
    {
        case CONFIG_TYPE_INTEGER:
        {
            value.toInt(&ok);
            return ok;
        }
        case CONFIG_TYPE_COLOR:
            return value.value<QColor>().isValid();
        case CONFIG_TYPE_LIST:
            return value.canConvert(QVariant::List);
        case CONFIG_TYPE_MAP:
            return value.canConvert(QVariant::Map);
    }

    return true;
}

} // end namespace
