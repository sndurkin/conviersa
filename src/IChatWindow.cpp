/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include "IChatWindow.h"

IChatWindow::IChatWindow(const QString &title/* = tr("Untitled")*/,
                         const QSize &size/* = QSize(500, 300)*/)
    : IWindow(title, size)
{ }
