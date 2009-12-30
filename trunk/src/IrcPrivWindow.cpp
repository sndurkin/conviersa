/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QWebView>
#include <QMutex>
#include "IrcTypes.h"
#include "IrcPrivWindow.h"
#include "IrcStatusWindow.h"
#include "Session.h"
#include "WindowManager.h"

namespace cv { namespace irc {

IrcPrivWindow::IrcPrivWindow(QExplicitlySharedDataPointer<Session> pSharedSession,
                QExplicitlySharedDataPointer<Connection> pSharedConn,
                const QString &title/* = tr("Untitled")*/,
                const QSize &size/* = QSize(500, 300)*/)
    : IIrcWindow(title, size)
{
    m_pSharedConn = pSharedConn;
    m_pSharedSession = pSharedSession;
    m_targetNick = title;

    m_pVLayout->addWidget(m_pOutput);
    m_pVLayout->addWidget(m_pInput);
    m_pVLayout->setSpacing(5);
    m_pVLayout->setContentsMargins(2, 2, 2, 2);
    setLayout(m_pVLayout);

    QObject::connect(m_pSharedConn.data(), SIGNAL(disconnected()), this, SLOT(handleDisconnect()));
}

int IrcPrivWindow::getIrcWindowType()
{
    return IRC_PRIV_WIN_TYPE;
}

// changes the nickname of the person we're chatting with
void IrcPrivWindow::setTargetNick(const QString &nick)
{
    m_targetNick = nick;
    setWindowName(nick);
    setTitle(nick);
}

// returns the target nick that we're chatting with
// (same as IWindow::GetWindowName() & IWindow::GetTitle())
QString IrcPrivWindow::getTargetNick()
{
    return m_targetNick;
}

void IrcPrivWindow::handleTab()
{
    QString text = getInputText();
    int idx = m_pInput->textCursor().position();
}

void IrcPrivWindow::closeEvent(QCloseEvent *event)
{
    emit privWindowClosing(this);
    return Window::closeEvent(event);
}

void IrcPrivWindow::handleDisconnect()
{
    printOutput("* Disconnected");
}

} } // end namespaces
