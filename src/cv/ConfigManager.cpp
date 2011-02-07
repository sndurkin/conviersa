/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QFile>
#include <QTemporaryFile>
#include "cv/ConfigManager.h"

namespace cv {

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
bool ConfigManager::setupConfigFile(const QString &filename, const QList<ConfigOption> &defOptions)
{
    QFile file(filename);
    if(file.exists() && !file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug("[CM::setupConfigFile] File %s could not be opened for read", filename.toLatin1().constData());
        return false;
    }

    // put all the default options into a QMap
    QMap<QString, QString>  options;
    for(int i = 0; i < defOptions.size(); ++i)
    {
        options.insert(defOptions[i].name, defOptions[i].value);
    }

    if(file.exists())
    {
        QRegExp newlineRegex("[\r\n]*");

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
            optValue.remove(newlineRegex);

            // find the option with key optName
            QMap<QString, QString>::iterator i = options.find(optName);
            if(i != options.end())
            {
                // if the value in the file is different from the default,
                // change the value
                if(i->compare(optValue) != 0)
                {
                    *i = optValue;
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

        for(int i = 0; i < defOptions.size(); ++i)
        {
            QByteArray str;
            str.append(defOptions[i].name);
            str.append('=');
            str.append(defOptions[i].value);
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
    QHash<QString, QMap<QString, QString> >::iterator i = m_files.find(filename);

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
                    QMap<QString, QString>::iterator j = i.value().find(optName);
                    if(j != i.value().end())
                    {
                        // write to newFile
                        QByteArray str;
                        str.append(optName);
                        str.append('=');
                        str.append(j.value());
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
        QMap<QString, QString>::iterator j = i.value().begin();
        while(j != i.value().end())
        {
            if(!alreadyWritten.contains(j.key()))
            {
                // write to newFile
                QByteArray str;
                str.append(j.key());
                str.append('=');
                str.append(j.value());
                str.append('\n');
                newFile.write(str);
            }

            ++j;
        }

        // copy the file
        newFile.copy(filename);
        newFile.remove();

        return true;
    }
    else
    {
        qDebug("[CM::writeToFile] Config file %s not found", filename.toLatin1().constData());
    }

    return false;
}

//-----------------------------------//

// returns the value of the provided option inside the provided file,
// returns an empty string upon error
QString ConfigManager::getOptionValue(const QString &filename, const QString &optName)
{
    QHash<QString, QMap<QString, QString> >::iterator i = m_files.find(filename);
    if(i != m_files.end())
    {
        QMap<QString, QString>::iterator j = i.value().find(optName);
        if(j != i.value().end())
        {
            return *j;
        }
        else
        {
            qDebug("[CM::getOptionValue] Config option %s (in file %s) does not exist", optName.toLatin1().constData(), filename.toLatin1().constData());
        }
    }
    else
    {
        qDebug("[CM::getOptionValue] Config file %s not found", filename.toLatin1().constData());
    }

    return "";
}

//-----------------------------------//

// sets the provided option's value to optValue
bool ConfigManager::setOptionValue(const QString &filename, const QString &optName, const QString &optValue)
{
    QHash<QString, QMap<QString, QString> >::iterator i = m_files.find(filename);
    if(i != m_files.end())
    {
        QMap<QString, QString>::iterator j = i.value().find(optName);
        if(j != i.value().end())
        {
            *j = optValue;
            return true;
        }
        else
        {
            qDebug("[CM::setOptionValue] Config option %s (in file %s) does not exist", optName.toLatin1().constData(), filename.toLatin1().constData());
        }
    }
    else
    {
        qDebug("[CM::setOptionValue] Config file %s not found", filename.toLatin1().constData());
    }

    return false;
}

//-----------------------------------//

// test function to produce contents of file
void ConfigManager::printFile(const QString &filename)
{
    QHash<QString, QMap<QString, QString> >::iterator i = m_files.find(filename);
    if(i != m_files.end())
    {
        QMap<QString, QString>::iterator j = i.value().begin();
        while(j != i.value().end())
        {
            printf("%s = %s\n", j.key().toAscii().data() , j.value().toAscii().data());
            ++j;
        }
    }
}

} // end namespace
