/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QString>
#include <QExplicitlySharedDataPointer>
#include "Window.h"
#include "Connection.h"

namespace cv {

class IChatWindow : public Window
{
protected:
    QExplicitlySharedDataPointer<Connection> m_pSharedConn;

public:
    IChatWindow(const QString &title = tr("Untitled"),
                const QSize &size = QSize(500, 300));

    virtual void handleData(QString &data) { }
};

} // end namespace
