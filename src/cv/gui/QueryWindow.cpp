/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QWebView>
#include <QMutex>
#include "cv/Session.h"
#include "cv/ConfigManager.h"
#include "cv/gui/WindowManager.h"
#include "cv/gui/QueryWindow.h"
#include "cv/gui/StatusWindow.h"

namespace cv { namespace gui {

QueryWindow::QueryWindow(QExplicitlySharedDataPointer<Session> pSharedSession,
                         const QString &targetNick)
    : InputOutputWindow(targetNick)
{
    m_pSharedSession = pSharedSession;
    m_targetNick = targetNick;

    m_pVLayout->addWidget(m_pOutput);
    m_pVLayout->addWidget(m_pInput);
    m_pVLayout->setSpacing(5);
    m_pVLayout->setContentsMargins(2, 2, 2, 2);
    setLayout(m_pVLayout);

    m_pSharedSession->getEventManager()->hookEvent("onNumericMessage", MakeDelegate(this, &QueryWindow::onNumericMessage));
    m_pSharedSession->getEventManager()->hookEvent("onNickMessage", MakeDelegate(this, &QueryWindow::onNickMessage));
    m_pSharedSession->getEventManager()->hookEvent("onPrivmsgMessage", MakeDelegate(this, &QueryWindow::onPrivmsgMessage));
}

QueryWindow::~QueryWindow()
{
    // todo: rewrite
    m_pSharedSession->getEventManager()->unhookEvent("onNumericMessage", MakeDelegate(this, &QueryWindow::onNumericMessage));
    m_pSharedSession->getEventManager()->unhookEvent("onNickMessage", MakeDelegate(this, &QueryWindow::onNickMessage));
    m_pSharedSession->getEventManager()->unhookEvent("onPrivmsgMessage", MakeDelegate(this, &QueryWindow::onPrivmsgMessage));
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

void QueryWindow::onNumericMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    switch(msg.m_command)
    {
        case 401:   // ERR_NOSUCKNICK
        case 404:   // ERR_CANNOTSENDTOCHAN
        {
            // msg.m_params[0]: my nick
            // msg.m_params[1]: nick/channel
            // msg.m_params[2]: "No such nick/channel"
            if(msg.m_params[1].compare(getWindowName(), Qt::CaseInsensitive) == 0)
            {
                printOutput(getNumericText(msg));
            }
        }
    }
}

void QueryWindow::onNickMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();

    // will print a nick change message to the PM window
    // if we get a NICK message, which will only be if we're in
    // a channel with the person (or if the nick being changed is ours)
    QString oldNick = parseMsgPrefix(msg.m_prefix, MsgPrefixName);
    QString textToPrint = QString("* %1 is now known as %2")
                          .arg(oldNick)
                          .arg(msg.m_params[0]);
    QString nickColor = g_pCfgManager->getOptionValue("colors.ini", "nick");
    if(m_pSharedSession->isMyNick(oldNick))
    {
        printOutput(textToPrint, nickColor);
    }
    else
    {
        // if the target nick has changed and there isn't another query with that name
        // already open, then we can safely change the this query's target nick
        bool queryWindowExists = dynamic_cast<StatusWindow *>(m_pManager->getParentWindow(this))->childIrcWindowExists(msg.m_params[0]);
        if(isTargetNick(oldNick) && !queryWindowExists)
        {
            setTargetNick(msg.m_params[0]);
            printOutput(textToPrint, nickColor);
        }
    }
}

void QueryWindow::onPrivmsgMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    if(m_pSharedSession->isMyNick(msg.m_params[0]))
    {
        QString fromNick = parseMsgPrefix(msg.m_prefix, MsgPrefixName);
        if(isTargetNick(fromNick))
        {
            QString textToPrint;
            QColor color;

            CtcpRequestType requestType = getCtcpRequestType(msg);
            if(requestType != RequestTypeInvalid)
            {
                // ACTION is /me, so handle according to that
                if(requestType == RequestTypeAction)
                {
                    QString action = msg.m_params[1];

                    // action = "\1ACTION <action>\1"
                    // first 8 characters and last 1 character need to be excluded
                    // so we'll take the mid, starting at index 8 and going until every
                    // character but the last is included
                    color.setNamedColor(g_pCfgManager->getOptionValue("colors.ini", "action"));
                    textToPrint = QString("* %1 %2")
                                  .arg(fromNick)
                                  .arg(action.mid(8, action.size()-9));
                }
            }
            else
            {
                color.setNamedColor(g_pCfgManager->getOptionValue("colors.ini", "say"));
                textToPrint = QString("<%1> %2")
                              .arg(fromNick)
                              .arg(msg.m_params[1]);
            }

            printOutput(textToPrint, color);
        }
    }
}

// handles the printing/sending of the PRIVMSG message
void QueryWindow::handleSay(const QString &text)
{
    QString textToPrint = QString("<%1> %2")
                          .arg(m_pSharedSession->getNick())
                          .arg(text);
    printOutput(textToPrint, QColor(g_pCfgManager->getOptionValue("colors.ini", "say")));
    m_pSharedSession->sendPrivmsg(getWindowName(), text);
}

// handles the printing/sending of the PRIVMSG ACTION message
void QueryWindow::handleAction(const QString &text)
{
    QString textToPrint = QString("* %1 %2")
                          .arg(m_pSharedSession->getNick())
                          .arg(text);
    printOutput(textToPrint, QColor(g_pCfgManager->getOptionValue("colors.ini", "action")));
    m_pSharedSession->sendAction(getWindowName(), text);
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
/*
void QueryWindow::onServerConnect() { }

void QueryWindow::onServerDisconnect()
{
    printOutput("* Disconnected");
}

void QueryWindow::onReceiveMessage(const Message &msg) { }
*/
} } // end namespaces
