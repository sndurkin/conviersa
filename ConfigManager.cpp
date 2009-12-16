#include <QFile>
#include <QTemporaryFile>
#include <QRegExp>
#include "ConfigManager.h"

// if the file with the given name exists, it checks for the setup
// provided by the array of ConfigOptions
//
// if they do not match, or no file with the given name exists,
// a default file is created with the given array of ConfigOptions
//
// once the file has been setup, the data it holds can be requested to
// be put into memory
bool ConfigManager::SetupConfigFile(const QString &filename, const QList<ConfigOption> &defOptions)
{
    QFile file(filename);
    if(file.exists() && !file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    // put all the default options into a QMap
    QMap<QString, QString>  options;
    for(int i = 0; i < defOptions.size(); ++i)
    {
        options.insert(defOptions[i].GetName(), defOptions[i].GetValue());
    }

    if(file.exists())
    {
        while(!file.atEnd())
        {
            QString line = file.readLine();

            // ignore comments (which start with '#')
            QRegExp regex("^\\s*#");
            if(line.contains(regex))
                continue;

            // if the line doesn't contain at least
            // one '=' or if the '=' is at the beginning
            // of the line, then it isn't of the right format
            // and will be skipped
            if(!line.contains('=', Qt::CaseInsensitive) || line[0] == '=')
                continue;

            QString optName = line.section('=', 0, 0);
            QString optValue = line.section('=', 1);
            optValue = optValue.trimmed();

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
            return false;

        for(int i = 0; i < defOptions.size(); ++i)
        {
            QByteArray str;
            str.append(defOptions[i].GetName());
            str.append('=');
            str.append(defOptions[i].GetValue());
            str.append('\n');
            file.write(str);
        }
    }

    file.close();
    m_files.insert(filename, options);
    return true;
}

// writes currently cached data to the file specified by filename
//
// returns true upon success,
// returns false otherwise
bool ConfigManager::WriteToFile(const QString &filename)
{
    QHash<QString, QMap<QString, QString> >::iterator i = m_files.find(filename);

    if(i != m_files.end())
    {
        QFile file(filename);
        if(file.exists() && !file.open(QIODevice::ReadOnly | QIODevice::Text))
            return false;

        // little hack i use to get a temporary filename
        // for creating with QFile so i can open it with
        // QIODevice::Text and not deal with the cross-platform
        // ugliness of newline characters
        QString tempFilename;
        {
            QTemporaryFile newTempFile;
            if(!newTempFile.open())
                return false;
            tempFilename = newTempFile.fileName();
            newTempFile.close();
        }

        QFile newFile(tempFilename);
        if(!newFile.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;

        QHash<QString, int>  alreadyWritten;

        if(file.exists())
        {
            while(!file.atEnd())
            {
                QString line = file.readLine();

                // automatically write everything that isn't
                // of the correct format
                QRegExp regex("^\\s*#");
                if(line.contains(regex))
                {
                    newFile.write(line.toAscii());
                }
                else if(!line.contains('=', Qt::CaseInsensitive) || line[0] == '=')
                {
                    // also makes it a comment, to show it was ignored
                    newFile.write("#" + line.toAscii());
                }
                else    // otherwise, we check the cached data
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

        // write any cached data that wasn't already in the file
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

    return false;
}

// returns the value of the provided option inside the provided file,
// returns an empty string upon error
QString ConfigManager::GetOptionValue(const QString &filename, const QString &optName)
{
    QHash<QString, QMap<QString, QString> >::iterator i = m_files.find(filename);
    if(i != m_files.end())
    {
        QMap<QString, QString>::iterator j = i.value().find(optName);
        if(j != i.value().end())
        {
            return *j;
        }
    }

    return "";
}

// sets the provided option's value to optValue
//
// returns true upon success,
// returns false otherwise
bool ConfigManager::SetOptionValue(const QString &filename, const QString &optName, const QString &optValue)
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
    }

    return false;
}

// test function to produce contents of file
void ConfigManager::PrintFile(const QString &filename)
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
