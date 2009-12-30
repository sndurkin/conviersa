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
#include "ConfigOption.h"

namespace cv {

class ConfigManager
{
    QHash<QString, QMap<QString, QString> > m_files;

    // comments start the line with '#'
    QRegExp m_commentRegex;

public:
    ConfigManager()
      : m_commentRegex("^\\s*#")
    { }

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
    bool setupConfigFile(const QString &filename, const QList<ConfigOption> &options);

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
    bool writeToFile(const QString &filename);

    // returns the value of the provided option inside the provided file,
    // returns an empty string upon error
    QString getOptionValue(const QString &filename, const QString &optName);

    // sets the provided option's value to optValue
    bool setOptionValue(const QString &filename, const QString &optName, const QString &optValue);

    // test function to produce contents of file
    void printFile(const QString &filename);
};

extern ConfigManager *g_pCfgManager;

} // end namespace
