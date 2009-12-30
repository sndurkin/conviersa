/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QObject>
#include <QString>

namespace cv { namespace irc {

// represents an IRC channel and can hold all the properties available
class Channel : public QObject
{
    Q_OBJECT

public:
    Channel();

private:
    QString name;
};

} } // end namespaces
