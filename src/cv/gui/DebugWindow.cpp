/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include "cv/gui/DebugWindow.h"
#include "cv/Session.h"

namespace cv { namespace gui {

DebugWindow::DebugWindow(Session *pSession,
                         const QString &title/* = tr("Debug Window")*/,
                         const QSize &size/* = QSize(500, 300)*/)
    : OutputWindow(title, size)
{
    m_pSession = pSession;

    m_pVLayout->addWidget(m_pOutput);
    m_pVLayout->setContentsMargins(2, 2, 2, 2);
    setLayout(m_pVLayout);

    g_pEvtManager->hookEvent("sendData", m_pSession, MakeDelegate(this, &DebugWindow::onSendData));
    g_pEvtManager->hookEvent("receivedData", m_pSession, MakeDelegate(this, &DebugWindow::onReceivedData));
}

DebugWindow::~DebugWindow()
{
    g_pEvtManager->unhookEvent("sendData", m_pSession, MakeDelegate(this, &DebugWindow::onSendData));
    g_pEvtManager->unhookEvent("receivedData", m_pSession, MakeDelegate(this, &DebugWindow::onReceivedData));
}

void DebugWindow::giveFocus() { }

void DebugWindow::onSendData(Event *pEvt)
{
    printOutput("<- " + DCAST(DataEvent, pEvt)->getData(), MESSAGE_INFO, COLOR_NONE, 1);
}

void DebugWindow::onReceivedData(Event *pEvt)
{
    printOutput("-> " + DCAST(DataEvent, pEvt)->getData().trimmed(), MESSAGE_INFO, COLOR_NONE, 1);
}

void DebugWindow::onOutput(Event *pEvt) { }
void DebugWindow::onDoubleClickLink(Event *pEvt) { }

} } // end namespaces
