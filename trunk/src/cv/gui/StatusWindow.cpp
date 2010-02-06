/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QWebView>
#include <QMutex>
#include <QDateTime>
#include "cv/qext.h"
#include "cv/Session.h"
#include "cv/ConfigManager.h"
#include "cv/gui/types.h"
#include "cv/gui/WindowManager.h"
#include "cv/gui/StatusWindow.h"
#include "cv/gui/ChannelListWindow.h"
#include "cv/gui/ChannelWindow.h"
#include "cv/gui/QueryWindow.h"
#include "cv/gui/OutputWindowScrollBar.h"

#define DEBUG_MESSAGES 0

namespace cv { namespace gui {

StatusWindow::StatusWindow(const QString &title/* = tr("Server Window")*/,
                           const QSize &size/* = QSize(500, 300)*/)
  : InputOutputWindow(title, size),
    m_populatingUserList(false),
    m_pChanListWin(NULL)
{
    m_pVLayout->addWidget(m_pOutput);
    m_pVLayout->addWidget(m_pInput);
    m_pVLayout->setSpacing(5);
    m_pVLayout->setContentsMargins(2, 2, 2, 2);
    setLayout(m_pVLayout);

    m_pSharedSession = new Session("conviersa");
    EventManager *pEvtMgr = m_pSharedSession->getEventManager();
    pEvtMgr->hookEvent("onConnect", MakeDelegate(this, &StatusWindow::onServerConnect));
    pEvtMgr->hookEvent("onDisconnect", MakeDelegate(this, &StatusWindow::onServerDisconnect));
    pEvtMgr->hookEvent("onReceiveData", MakeDelegate(this, &StatusWindow::onReceiveData));
    pEvtMgr->hookEvent("onErrorMessage", MakeDelegate(this, &StatusWindow::onErrorMessage));
    pEvtMgr->hookEvent("onInviteMessage", MakeDelegate(this, &StatusWindow::onInviteMessage));
    pEvtMgr->hookEvent("onJoinMessage", MakeDelegate(this, &StatusWindow::onJoinMessage));
    pEvtMgr->hookEvent("onModeMessage", MakeDelegate(this, &StatusWindow::onModeMessage));
    pEvtMgr->hookEvent("onNickMessage", MakeDelegate(this, &StatusWindow::onNickMessage));
    pEvtMgr->hookEvent("onNoticeMessage", MakeDelegate(this, &StatusWindow::onNoticeMessage));
    pEvtMgr->hookEvent("onPongMessage", MakeDelegate(this, &StatusWindow::onPongMessage));
    pEvtMgr->hookEvent("onPrivmsgMessage", MakeDelegate(this, &StatusWindow::onPrivmsgMessage));
    pEvtMgr->hookEvent("onQuitMessage", MakeDelegate(this, &StatusWindow::onQuitMessage));
    pEvtMgr->hookEvent("onWallopsMessage", MakeDelegate(this, &StatusWindow::onWallopsMessage));
    pEvtMgr->hookEvent("onNumericMessage", MakeDelegate(this, &StatusWindow::onNumericMessage));
    pEvtMgr->hookEvent("onUnknownMessage", MakeDelegate(this, &StatusWindow::onUnknownMessage));
}

StatusWindow::~StatusWindow()
{
    //QObject::disconnect(m_pSharedSession.data(), 0, 0, 0);
    m_pSharedSession->disconnectFromServer();
}

int StatusWindow::getIrcWindowType()
{
    return IRC_STATUS_WIN_TYPE;
}

// returns a pointer to the OutputWindow if it exists
// 	(and is a child of this status window)
// returns NULL otherwise
OutputWindow *StatusWindow::getChildIrcWindow(const QString &name)
{
    for(int i = 0; i < m_chanList.size(); ++i)
    {
        if(name.compare(m_chanList[i]->getWindowName(), Qt::CaseInsensitive) == 0)
        {
            return m_chanList[i];
        }
    }

    for(int i = 0; i < m_privList.size(); ++i)
    {
        if(name.compare(m_privList[i]->getWindowName(), Qt::CaseInsensitive) == 0)
        {
            return m_privList[i];
        }
    }

    return NULL;
}

// returns true if the child window with the provided name
// exists, returns false otherwise
bool StatusWindow::childIrcWindowExists(const QString &name)
{
    return (getChildIrcWindow(name) != NULL);
}

// returns a list of all IrcChanWindows that are currently
// being managed in the server
QList<ChannelWindow *> StatusWindow::getChannels()
{
    return m_chanList;
}

// returns a list of all IrcPrivWindows that are currently
// being managed in the server
QList<QueryWindow *> StatusWindow::getPrivateMessages()
{
    return m_privList;
}

// adds a channel to the list
void StatusWindow::addChannelWindow(ChannelWindow *pChanWin)
{
    if(m_pManager)
        m_pManager->addWindow(pChanWin, m_pManager->getItemFromWindow(this));
    m_chanList.append(pChanWin);
    QObject::connect(pChanWin, SIGNAL(chanWindowClosing(ChannelWindow *)),
                this, SLOT(removeChannelWindow(ChannelWindow *)));
}

// removes a channel from the list
void StatusWindow::removeChannelWindow(ChannelWindow *pChanWin)
{
    m_chanList.removeOne(pChanWin);
}

// adds a PM window to the list
void StatusWindow::addQueryWindow(QueryWindow *pQueryWin)
{
    if(m_pManager)
        m_pManager->addWindow(pQueryWin, m_pManager->getItemFromWindow(this));
    m_privList.append(pQueryWin);
    QObject::connect(pQueryWin, SIGNAL(privWindowClosing(QueryWindow *)),
                this, SLOT(removeQueryWindow(QueryWindow *)));
}

// removes a PM window from the list
void StatusWindow::removeQueryWindow(QueryWindow *pPrivWin)
{
    m_privList.removeOne(pPrivWin);
}

void StatusWindow::handleTab()
{
    QString text = getInputText();
    int idx = m_pInput->textCursor().position();
}

// handles child widget events
bool StatusWindow::eventFilter(QObject *obj, QEvent *event)
{
    // just for monitoring when it's closed
    if(obj == m_pChanListWin && event->type() == QEvent::Close)
    {
        m_pChanListWin = NULL;
    }

    return InputOutputWindow::eventFilter(obj, event);
}

void StatusWindow::handle321Numeric(const Message &msg)
{
    if(!m_pChanListWin)
    {
        m_pChanListWin = new (std::nothrow) ChannelListWindow(m_pSharedSession);
        if(!m_pChanListWin)
            return;

        m_pManager->addWindow(m_pChanListWin, m_pManager->getItemFromWindow(this));
        QString title = QString("Channel List - %1").arg(getWindowName());
        m_pChanListWin->setTitle(title);

        // todo: fix and put inside my own event system
        m_pChanListWin->installEventFilter(this);
    }
    else
    {
        m_pChanListWin->clearList();
        QString title = QString("Channel List - %1").arg(getWindowName());
        m_pChanListWin->setTitle(title);
    }

    m_pChanListWin->beginPopulatingList();
}

void StatusWindow::handle322Numeric(const Message &msg)
{
    // msg.m_params[0]: my nick
    // msg.m_params[1]: channel
    // msg.m_params[2]: number of users
    // msg.m_params[3]: topic
    if(m_pChanListWin)
    {
        m_pChanListWin->addChannel(msg.m_params[1], msg.m_params[2], msg.m_params[3]);
    }
    else
    {
        if(!m_sentListStopMsg)
        {
            m_sentListStopMsg = true;
            m_pSharedSession->sendData("LIST STOP");
        }
    }
}

void StatusWindow::handle323Numeric(const Message &msg)
{
    if(m_pChanListWin)
    {
        m_pChanListWin->endPopulatingList();
    }

    m_sentListStopMsg = false;
}

void StatusWindow::handle353Numeric(const Message &msg)
{
    // msg.m_params[0]: my nick
    // msg.m_params[1]: "=" | "*" | "@"
    // msg.m_params[2]: channel
    // msg.m_params[3]: names, separated by spaces
    ChannelWindow *pChanWin = dynamic_cast<ChannelWindow *>(getChildIrcWindow(msg.m_params[2]));

    // RPL_NAMREPLY was sent as a result of a JOIN command
    if(pChanWin && m_populatingUserList)
    {
        int numSections = msg.m_params[3].count(' ') + 1;
        for(int i = 0; i < numSections; ++i)
        {
            pChanWin->addUser(msg.m_params[3].section(' ', i, i, QString::SectionSkipEmpty));
        }
    }
    // RPL_NAMREPLY was sent as a result of a NAMES command
    else
    {
        printOutput(getNumericText(msg));
    }
}

void StatusWindow::handle366Numeric(const Message &msg)
{
    // msg.m_params[0]: my nick
    // msg.m_params[1]: channel
    // msg.m_params[2]: "End of NAMES list"

    // RPL_ENDOFNAMES was sent as a result of a NAMES command
    if(!childIrcWindowExists(msg.m_params[1]))
    {
        //printOutput(ConvertDataToHtml(getNumericText(msg)));
        printOutput(getNumericText(msg));
    }
}

void StatusWindow::onServerConnect(Event *evt) { }

void StatusWindow::onServerDisconnect(Event *evt)
{
    printOutput("* Disconnected");
    setTitle("Server Window");
    setWindowName("Server Window");
}

void StatusWindow::onReceiveData(Event *evt)
{
#if DEBUG_MESSAGES
    QString data = dynamic_cast<DataEvent *>(evt)->getData();
    data.remove(data.size()-2,2);
    printDebug(data);
#endif
}

void StatusWindow::onNumericMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    switch(msg.m_command)
    {
        case 1:
        {
            // change name of the window to the name of the network
            QString networkName = getNetworkNameFrom001(msg);
            setTitle(networkName);
            setWindowName(networkName);

            break;
        }
        // RPL_AWAY
        case 301:
        {
            // msg.m_params[0]: my nick
            // msg.m_params[1]: nick
            // msg.m_params[2]: away message
            QString textToPrint = QString("%1 is away: %2")
                                  .arg(msg.m_params[1])
                                  .arg(convertDataToHtml(msg.m_params[2]));
            printOutput(textToPrint);

            break;
        }
        /*// RPL_USERHOST
        case 302:
        {

            break;
        }
        // RPL_ISON
        case 303:
        {

            break;
        }*/
        // RPL_WHOISIDLE
        case 317:
        {
            QString textToPrint = QString("%1 has been idle %2")
                                  .arg(msg.m_params[1])
                                  .arg(getIdleTextFrom317(msg));
            printOutput(textToPrint);
            break;
        }
        // RPL_LISTSTART
        case 321:
        {
            handle321Numeric(msg);
            break;
        }
        // RPL_LIST
        case 322:
        {
            handle322Numeric(msg);
            break;
        }
        // RPL_LISTEND
        case 323:
        {
            handle323Numeric(msg);
            break;
        }
        // RPL_WHOISACCOUNT
        case 330:
        {
            // msg.m_params[0]: my nick
            // msg.m_params[1]: nick
            // msg.m_params[2]: login/auth
            // msg.m_params[3]: "is logged in as"
            QString textToPrint = QString("%1 %2: %3")
                                  .arg(msg.m_params[1])
                                  .arg(msg.m_params[3])
                                  .arg(msg.m_params[2]);
            printOutput(textToPrint);
            break;
        }
        // RPL_TOPIC
        case 332:
        {
            // msg.m_params[0]: my nick
            // msg.m_params[1]: channel
            // msg.m_params[2]: topic
            if(!childIrcWindowExists(msg.m_params[1]))
            {
                printOutput(getNumericText(msg));
            }
            break;
        }
        // states when topic was last set
        case 333:
        {
            // msg.m_params[0]: my nick
            // msg.m_params[1]: channel
            // msg.m_params[2]: nick
            // msg.m_params[3]: unix time
            if(!childIrcWindowExists(msg.m_params[1]))
            {
                QString textToPrint = QString("%1 topic set by %2 on %3 %4")
                                      .arg(msg.m_params[1])
                                      .arg(msg.m_params[2])
                                      .arg(getDate(msg.m_params[3]))
                                      .arg(getTime(msg.m_params[3]));
                printOutput(textToPrint);
            }

            break;
        }
        // RPL_NAMREPLY
        case 353:
        {
            handle353Numeric(msg);
            break;
        }
        // RPL_ENDOFNAMES
        case 366:
        {
            handle366Numeric(msg);
            break;
        }
        case 401:   // ERR_NOSUCKNICK
        case 404:   // ERR_CANNOTSENDTOCHAN
        {
            // msg.m_params[0]: my nick
            // msg.m_params[1]: nick/channel
            // msg.m_params[2]: "No such nick/channel"
            if(!childIrcWindowExists(msg.m_params[1]))
            {
                printOutput(getNumericText(msg));
            }
            break;
        }
        default:
        {
            // the following are not meant to be handled,
            // but only printed:
            // 	003
            // 	004
            // 	305
            // 	306
            //printOutput(convertDataToHtml(getNumericText(msg)));
            printOutput(getNumericText(msg));
        }
    }
}

void StatusWindow::onErrorMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    printOutput(msg.m_params[0]);
}

void StatusWindow::onInviteMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    QString textToPrint = QString("* %1 has invited you to %2")
                          .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName))
                          .arg(msg.m_params[1]);
    printOutput(textToPrint, QColor(g_pCfgManager->getOptionValue("colors.ini", "invite")));
}

void StatusWindow::onJoinMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();

    QString nickJoined = parseMsgPrefix(msg.m_prefix, MsgPrefixName);
    if(m_pSharedSession->isMyNick(nickJoined) && !childIrcWindowExists(msg.m_params[0]))
    {
        // create the channel and post the message to it
        ChannelWindow *pChanWin = new (std::nothrow) ChannelWindow(m_pSharedSession, msg.m_params[0]);
        if(!pChanWin)
        {
            printError("Allocation of a new channel window failed.");
            return;
        }
        addChannelWindow(pChanWin);
        pChanWin->joinChannel();
        m_populatingUserList = true;
        QString textToPrint = QString("* You have joined %1").arg(msg.m_params[0]);
        pChanWin->printOutput(textToPrint, QColor(g_pCfgManager->getOptionValue("colors.ini", "join")));
    }
}

void StatusWindow::onModeMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    if(!childIrcWindowExists(msg.m_params[0]))  // user mode
    {
        QString textToPrint = QString("* %1 has set mode: ").arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName));

        // ignore first parameter
        for(int i = 1; i < msg.m_paramsNum; ++i)
        {
                textToPrint += msg.m_params[i];
                textToPrint += ' ';
        }

        printOutput(textToPrint, QColor(g_pCfgManager->getOptionValue("colors.ini", "mode")));
    }
}

void StatusWindow::onNickMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();

    QString oldNick = parseMsgPrefix(msg.m_prefix, MsgPrefixName);
    if(m_pSharedSession->isMyNick(oldNick))
    {
        QString textToPrint = QString("* %1 is now known as %2")
                              .arg(oldNick)
                              .arg(msg.m_params[0]);
        printOutput(textToPrint, g_pCfgManager->getOptionValue("colors.ini", "nick"));
    }
}

void StatusWindow::onNoticeMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    QString source;
    if(!msg.m_prefix.isEmpty())
    {
        source = parseMsgPrefix(msg.m_prefix, MsgPrefixName);
    }
    // if m_prefix is empty, it is from the host
    else
    {
        source = m_pSharedSession->getHost();
    }

    QString textToPrint = QString("-%1- %2")
                          .arg(source)
                          .arg(msg.m_params[1]);
    printOutput(textToPrint, g_pCfgManager->getOptionValue("colors.ini", "notice"));
}

void StatusWindow::onPongMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();

    // the prefix is used to determine the server that
    // sends the PONG rather than the first parameter,
    // because it will always have the server in it
    //
    // example:
    //	PING hi :there
    //	:irc.server.net PONG there :hi
    QString textToPrint = QString("* PONG from %1: %2")
                          .arg(msg.m_prefix)
                          .arg(msg.m_params[1]);
    printOutput(textToPrint);
}

void StatusWindow::onPrivmsgMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();

    QString fromNick = parseMsgPrefix(msg.m_prefix, MsgPrefixName);
    CtcpRequestType requestType = getCtcpRequestType(msg);
    if(requestType != RequestTypeInvalid &&
       requestType != RequestTypeAction)
    {
        QString replyStr;
        QString requestTypeStr;

        switch(requestType)
        {
            case RequestTypeVersion:
            {
                replyStr = "conviersa v0.00000000001";
                requestTypeStr = "VERSION";
                break;
            }
            case RequestTypeTime:
            {
                replyStr = "bitch, please";
                requestTypeStr = "TIME";
                break;
            }
            case RequestTypeFinger:
            {
                replyStr = "good hustle.";
                requestTypeStr = "FINGER";
                break;
            }
            default:
            {
                requestTypeStr = "UNKNOWN";
            }
        }

        if(!replyStr.isEmpty())
        {
            QString textToSend = QString("NOTICE %1 :\1%2 %3\1")
                                 .arg(fromNick)
                                 .arg(requestTypeStr)
                                 .arg(replyStr);
            m_pSharedSession->sendData(textToSend);
        }

        QString textToPrint = QString("[CTCP %1 (from %2)]")
                              .arg(requestTypeStr)
                              .arg(fromNick);
        printOutput(textToPrint, g_pCfgManager->getOptionValue("colors.ini", "ctcp"));
        return;
    }

    // if the target is me, then it's a PM from someone
    if(m_pSharedSession->isMyNick(msg.m_params[0]) && !childIrcWindowExists(fromNick))
    {
        QueryWindow *pQueryWin = new QueryWindow(m_pSharedSession, fromNick);
        addQueryWindow(pQueryWin);

        // delegate to  window
        pQueryWin->onPrivmsgMessage(evt);
    }
}

void StatusWindow::onQuitMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    QString user = parseMsgPrefix(msg.m_prefix, MsgPrefixName);
    QString textToPrint = QString("* %1 (%2) has quit")
                            .arg(user)
                            .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixUserAndHost));

    QColor quitColor(g_pCfgManager->getOptionValue("colors.ini", "quit"));
    bool hasReason = (msg.m_paramsNum > 0 && !msg.m_params[0].isEmpty());
    if(hasReason)
    {
        textToPrint += QString(" (%1)").arg(msg.m_params[0]);
    }

    /*
    if(hasReason)
    {
        textToPrint += QString("</font><font color=%1>)</font>").arg(quitColor.name());
    }
    */
    for(int i = 0; i < m_chanList.size(); ++i)
    {
        if(m_chanList[i]->hasUser(user))
        {
            m_chanList[i]->removeUser(user);
            m_chanList[i]->printOutput(textToPrint, quitColor);
        }
    }

    // will print a quit message to the private message window
    // if we get a QUIT message, which will only be if we're in
    // a channel with the person
    for(int i = 0; i < m_privList.size(); ++i)
    {
        if(user.compare(m_privList[i]->getTargetNick(), Qt::CaseInsensitive) == 0)
        {
            m_privList[i]->printOutput(textToPrint, quitColor);
            break;
        }
    }
}

void StatusWindow::onWallopsMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    QString textToPrint = QString("* WALLOPS from %1: %2")
                .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName))
                .arg(msg.m_params[0]);
    printOutput(textToPrint);
}

void StatusWindow::onUnknownMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    // print the whole raw line
    //printOutput(data);
    // todo: decide what to do here
}

} } // end namespaces
