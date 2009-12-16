#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QHash>
#include <QString>
#include <QMap>
#include <QList>
#include "ConfigOption.h"

class ConfigManager
{
    QHash<QString, QMap<QString, QString> >    m_files;

public:
    // if the file with the given name exists, it checks for the setup
    // provided by the array of ConfigOptions
    //
    // if they do not match, or no file with the given name exists,
    // a default file is created with the given array of ConfigOptions
    //
    // once the file has been setup, the data it holds can be requested to
    // be put into memory
    bool SetupConfigFile(const QString &filename, const QList<ConfigOption> &options);

    // writes currently cached data to the file specified by filename
    //
    // returns true upon success,
    // returns false otherwise
    bool WriteToFile(const QString &filename);

    // returns the value of the provided option inside the provided file,
    // returns an empty string upon error
    QString GetOptionValue(const QString &filename, const QString &optName);

    // sets the provided option's value to optValue
    //
    // returns true upon success,
    // returns false otherwise
    bool SetOptionValue(const QString &filename, const QString &optName, const QString &optValue);

    // test function to produce contents of file
    void PrintFile(const QString &filename);
};

extern ConfigManager *g_pCfgManager;

#endif
