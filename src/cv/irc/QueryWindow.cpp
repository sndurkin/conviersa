/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QWebView>
#include <QMutex>
#include "irc/Session.h"
#include "cv/WindowManager.h"
#include "cv/irc/types.h"
#include "cv/irc/QueryWindow.h"
#include "cv/irc/StatusWindow.h"

namespace cv { namespace irc {

QueryWindow::QueryWindow(QExplicitlySharedDataPointer<Session> pSharedSession,
                         const QString &title/* = tr("Untitled")*/,
                         const QSize &size/* = QSize(500, 300)*/)
    : OutputWindow(title, size)
{
    m_pSharedSession = pSharedSession;
    m_targetNick = title;

    m_pVLayout->addWidget(m_pOutput);
    m_pVLayout->addWidget(m_pInput);
    m_pVLayout->setSpacing(5);
    m_pVLayout->setContentsMargins(2, 2, 2, 2);
    setLayout(m_pVLayout);

    QObject::connect(m_pSharedSession.data(), SIGNAL(connected()), this, SLOT(onServerConnect()));
    QObject::connect(m_pSharedSession.data(), SIGNAL(disconnected()), this, SLOT(onServerDisconnect()));
    //QObject::connect(m_pSharedSession.data(), SIGNAL(dataParsed(Message)), this, SLOT(onReceiveMessage(Message)));
}

QueryWindow::~QueryWindow()
{
    QObject::disconnect(m_pSharedSession.data(), 0, this, 0);
}

int QueryWindow::getIrcWindowType()
{
    return IRC_PRIV_WIN_TYPE;
}

// changes the nickname of the person we're chatting with
void QueryWindow::setTargetNick(const QString &nick)
{
    m_targetNick = nick;
    setWindowName(nick);
    setTitle(nick);
}

// returns the target nick that we're chatting with
// (same as IWindow::GetWindowName() & IWindow::GetTitle())
QString QueryWindow::getTargetNick()
{
    return m_targetNick;
}

void QueryWindow::handleTab()
{
    QString text = getInputText();
    int idx = m_pInput->textCursor().position();
}

void QueryWindow::closeEvent(QCloseEvent *event)
{
    emit privWindowClosing(this);
    return Window::closeEvent(event);
}

void QueryWindow::onServerConnect() { }

void QueryWindow::onServerDisconnect()
{
    printOutput("* Disconnected");
}

void QueryWindow::onReceiveMessage(const Message &msg) { }

} } // end namespaces
