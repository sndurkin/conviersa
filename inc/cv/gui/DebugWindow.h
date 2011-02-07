/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include "cv/gui/OutputWindow.h"

namespace cv { namespace gui {

class DebugWindow : public OutputWindow
{
public:
    DebugWindow(Session *pSession,
                const QString &title = tr("Debug Window"),
                const QSize &size = QSize(500, 300));
    ~DebugWindow();

    void giveFocus() { }

    void onSendData(Event *pEvt);
    void onReceivedData(Event *pEvt);

    void onOutput(Event *pEvt);
    void onDoubleClickLink(Event *pEvt);
};

} } // end namespaces
