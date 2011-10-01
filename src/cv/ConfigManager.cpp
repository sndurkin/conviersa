/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QFile>
#include <QTemporaryFile>
#include <QColor>
#include "cv/ConfigManager.h"

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
        // for each line in the file, find the corresponding option;
        // if the option exists in memory, overwrite it with
        // the value in the file
        while(!file.atEnd())
        {
            QString line = file.readLine();

            // comments are ignored
            if(line.contains(m_commentRegex))
                continue;

            // if the line doesn't contain at least
            // one '=' or if the '=' is at the beginning
            // of the line, then it isn't of the right format
            // and will be skipped
            if(!line.contains('=', Qt::CaseInsensitive) || line[0] == '=')
                continue;

            QString optName = line.section('=', 0, 0);
            QString optValue = line.section('=', 1);
            optValue.remove(m_newlineRegex);

            // find the option with key optName
            QMap<QString, ConfigOption>::iterator i = options.find(optName);
            if(i != options.end())
            {
                // if the value in the file is different from the default
                if(i->value.compare(optValue) != 0)
                {
                    // and the value in the file is valid
                    if(isValueValid(optValue, i->type))
                    {
                        // then change the value
                        i->value = optValue;
                    }
                    else
                        qDebug("[CM::setupConfigFile] Value %s is not valid for config option %s",
                               optValue.toLatin1().constData(),
                               optName.toLatin1().constData());
                }
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
        for(QMap<QString, ConfigOption>::iterator i = options.begin(); i != options.end(); ++i)
        {
            QByteArray str;
            str.append(i.key());
            str.append('=');
            str.append(i->value);
            str.append('\n');
            file.write(str);
        }
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
        if(file.exists() && !file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug("[CM::writeToFile] File %s could not be opened for read", filename.toLatin1().constData());
            return false;
        }

        // little hack i use to get a temporary filename
        // for creating with QFile so i can open it with
        // QIODevice::Text and not deal with the cross-platform
        // ugliness of newline characters
        QString tempFilename;
        {
            QTemporaryFile newTempFile;
            if(!newTempFile.open())
            {
                qDebug("[CM::writeToFile] Temporary file could not be opened", filename.toLatin1().constData());
                return false;
            }
            tempFilename = newTempFile.fileName();
            newTempFile.close();
        }

        QFile newFile(tempFilename);
        if(!newFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug("[CM::writeToFile] File %s could not be opened for write", filename.toLatin1().constData());
            return false;
        }

        QHash<QString, int>  alreadyWritten;

        if(file.exists())
        {
            while(!file.atEnd())
            {
                QString line = file.readLine();

                // automatically write everything that isn't
                // of the correct format
                if(line.contains(m_commentRegex))
                {
                    newFile.write(line.toAscii());
                }
                else if(!line.contains('=', Qt::CaseInsensitive) || line[0] == '=')
                {
                    // also makes it a comment, to show it was ignored
                    newFile.write("#" + line.toAscii());
                }
                else    // otherwise, we check the data in memory
                {
                    QString optName = line.section('=', 0, 0);
                    QMap<QString, ConfigOption>::iterator j = i.value().find(optName);
                    if(j != i.value().end())
                    {
                        // write to newFile
                        QByteArray str;
                        str.append(optName);
                        str.append('=');
                        str.append(j->value);
                        str.append('\n');
                        newFile.write(str);

                        // add it to the hash of already written strings
                        alreadyWritten.insert(optName, 0);
                    }
                    else    // if there's some problem, write the same line
                    {
                        newFile.write(line.toAscii());
                    }
                }
            }

            file.remove();
        }

        // write any data in memory that wasn't already in the file
        // (in other words, any data that isn't found in the
        // alreadyWritten hash table)
        QMap<QString, ConfigOption>::iterator j = i.value().begin();
        for(; j != i.value().end(); ++j)
        {
            if(!alreadyWritten.contains(j.key()))
            {
                // write to newFile
                QByteArray str;
                str.append(j.key());
                str.append('=');
                str.append(j->value);
                str.append('\n');
                newFile.write(str);
            }
        }

        // copy the file
        newFile.copy(filename);
        newFile.remove();

        return true;
    }
    else
        qDebug("[CM::writeToFile] Config file %s not found", filename.toLatin1().constData());

    return false;
}

//-----------------------------------//

// returns the value of the provided option inside the provided file,
// returns an empty string upon error
QString ConfigManager::getOptionValue(const QString &filename, const QString &optName)
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
        const QString &optValue,
        bool fireEvent/* = false*/)
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
                qDebug("[CM::setOptionValue] Value %s is not valid for config option %s",
                       optValue.toLatin1().constData(),
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
bool ConfigManager::isValueValid(const QString &value, ConfigType type)
{
    bool ok;
    switch(type)
    {
        case CONFIG_TYPE_INTEGER:
        {
            value.toInt(&ok);
            return ok;
        }
        case CONFIG_TYPE_BOOLEAN:
        {
            int val = value.toInt(&ok);
            return (val == 0 || val == 1);
        }
        case CONFIG_TYPE_COLOR:
            return QColor::isValidColor(value);
    }

    return true;
}

//-----------------------------------//

// test function to produce contents of file
void ConfigManager::printFile(const QString &filename)
{
    QHash<QString, QMap<QString, ConfigOption> >::iterator i = m_files.find(filename);
    if(i != m_files.end())
    {
        QMap<QString, ConfigOption>::iterator j = i.value().begin();
        for(; j != i.value().end(); ++j)
        {
            printf("%s = %s\n", j.key().toAscii().data(), j->value.toAscii().data());
        }
    }
}

} // end namespace
