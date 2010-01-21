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
  : OutputWindow(title, size),
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
    pEvtMgr->HookEvent("onConnect", MakeDelegate(this, &StatusWindow::onServerConnect));
    pEvtMgr->HookEvent("onDisconnect", MakeDelegate(this, &StatusWindow::onServerDisconnect));
    pEvtMgr->HookEvent("onReceiveData", MakeDelegate(this, &StatusWindow::onReceiveData));
    pEvtMgr->HookEvent("onErrorMessage", MakeDelegate(this, &StatusWindow::onErrorMessage));
    pEvtMgr->HookEvent("onInviteMessage", MakeDelegate(this, &StatusWindow::onInviteMessage));
    pEvtMgr->HookEvent("onJoinMessage", MakeDelegate(this, &StatusWindow::onJoinMessage));
    pEvtMgr->HookEvent("onModeMessage", MakeDelegate(this, &StatusWindow::onModeMessage));
    pEvtMgr->HookEvent("onNickMessage", MakeDelegate(this, &StatusWindow::onNickMessage));
    pEvtMgr->HookEvent("onNoticeMessage", MakeDelegate(this, &StatusWindow::onNoticeMessage));
    pEvtMgr->HookEvent("onPartMessage", MakeDelegate(this, &StatusWindow::onPartMessage));
    pEvtMgr->HookEvent("onPongMessage", MakeDelegate(this, &StatusWindow::onPongMessage));
    pEvtMgr->HookEvent("onPrivmsgMessage", MakeDelegate(this, &StatusWindow::onPrivmsgMessage));
    pEvtMgr->HookEvent("onQuitMessage", MakeDelegate(this, &StatusWindow::onQuitMessage));
    pEvtMgr->HookEvent("onTopicMessage", MakeDelegate(this, &StatusWindow::onTopicMessage));
    pEvtMgr->HookEvent("onWallopsMessage", MakeDelegate(this, &StatusWindow::onWallopsMessage));
    pEvtMgr->HookEvent("onNumericMessage", MakeDelegate(this, &StatusWindow::onNumericMessage));
    pEvtMgr->HookEvent("onUnknownMessage", MakeDelegate(this, &StatusWindow::onUnknownMessage));
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

    return OutputWindow::eventFilter(obj, event);
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
    OutputWindow *pChanWin = getChildIrcWindow(msg.m_params[1]);

    // RPL_ENDOFNAMES was sent as a result of a JOIN command
    if(pChanWin && m_populatingUserList)
    {
        m_populatingUserList = false;
    }
    // RPL_ENDOFNAMES was sent as a result of a NAMES command
    else
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
            if(getChildIrcWindow(msg.m_params[1]) == NULL)
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
            if(getChildIrcWindow(msg.m_params[1]) == NULL)
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
            if(getChildIrcWindow(msg.m_params[1]) == NULL)
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
    ChannelWindow *pChanWin = dynamic_cast<ChannelWindow *>(getChildIrcWindow(msg.m_params[0]));
    QString textToPrint = "* ";

    // if the JOIN message is of you joining the channel
    QString nickJoined = parseMsgPrefix(msg.m_prefix, MsgPrefixName);
    if(m_pSharedSession->getNick().compare(nickJoined, Qt::CaseInsensitive) == 0)
    {
        if(pChanWin)
        {
            textToPrint += "You have rejoined ";
        }
        else
        {
            // create the channel and post the message to it
            pChanWin = new (std::nothrow) ChannelWindow(m_pSharedSession, msg.m_params[0]);
            if(!pChanWin)
            {
                printError("Allocation of a new channel window failed.");
                return;
            }
            addChannelWindow(pChanWin);
            textToPrint += "You have joined ";
        }

        pChanWin->joinChannel();
        textToPrint += msg.m_params[0];
        m_populatingUserList = true;
    }
    else
    {
        if(pChanWin)
        {
            textToPrint += QString("%1 (%2) has joined %3")
                    .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName))
                    .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixUserAndHost))
                    .arg(msg.m_params[0]);
            pChanWin->addUser(nickJoined);
        }
    }

    if(pChanWin)
        pChanWin->printOutput(textToPrint, QColor(g_pCfgManager->getOptionValue("colors.ini", "join")));
    else
        printError("Pointer to channel window is invalid. This path should not have been reached.");
}

void StatusWindow::onKickMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    ChannelWindow *pChanWin = dynamic_cast<ChannelWindow *>(getChildIrcWindow(msg.m_params[0]));
    if(!pChanWin)
        return;

    QString textToPrint;

    // if the KICK message is for you
    if(m_pSharedSession->getNick().compare(msg.m_params[1]) == 0)
    {
        pChanWin->leaveChannel();
        textToPrint = "* You were kicked from ";
    }
    else
    {
        textToPrint = QString("* %1 was kicked from ").arg(msg.m_params[1]);
        pChanWin->removeUser(msg.m_params[1]);
    }

    textToPrint += QString("%1 by %2")
                .arg(msg.m_params[0])
                .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName));

    bool hasReason = !msg.m_params[2].isEmpty();
    if(hasReason)
    {
        textToPrint += QString(" (%1)").arg(msg.m_params[2]);
    }

    QColor kickColor(g_pCfgManager->getOptionValue("colors.ini", "kick"));
    /*
    if(hasReason)
    {
        textToPrint += QString("</font><font color=%1>)</font>").arg(kickColor.name());
    }
    */

    pChanWin->printOutput(textToPrint, kickColor);
}

void StatusWindow::onModeMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    QString textToPrint = QString("* %1 has set mode: ").arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName));

    // ignore first parameter
    for(int i = 1; i < msg.m_paramsNum; ++i)
    {
            textToPrint += msg.m_params[i];
            textToPrint += ' ';
    }

    // channel mode
    //
    // todo: fix
    if(isChannel(msg.m_params[0]))
    {
        ChannelWindow *pChanWin = dynamic_cast<ChannelWindow *>(getChildIrcWindow(msg.m_params[0]));
        if(!pChanWin)
            return;

        bool sign = true;
        QString modes = msg.m_params[1];

        for(int modesIndex = 0, paramsIndex = 2; modesIndex < modes.size(); ++modesIndex)
        {
            if(modes[modesIndex] == '+')
            {
                sign = true;
            }
            else if(modes[modesIndex] == '-')
            {
                sign = false;
            }
            else
            {
                ChanModeType type = getChanModeType(m_pSharedSession->getChanModes(), modes[modesIndex]);
                switch(type)
                {
                    case ModeTypeA:
                    case ModeTypeB:
                    case ModeTypeC:
                    {
                        // if there's no params left then continue
                        if(paramsIndex >= msg.m_paramsNum)
                            break;

                        QChar prefix = m_pSharedSession->getPrefixRule(modes[modesIndex]);
                        if(prefix != '\0')
                        {
                            if(sign)
                                pChanWin->addPrefixToUser(msg.m_params[paramsIndex], prefix);
                            else
                                pChanWin->removePrefixFromUser(msg.m_params[paramsIndex], prefix);
                        }

                        ++paramsIndex;
                        break;
                    }
                    default:
                    {

                    }
                }
            }
        }

        pChanWin->printOutput(textToPrint, QColor(g_pCfgManager->getOptionValue("colors.ini", "mode")));
    }
    else	// user mode
    {
        printOutput(textToPrint, QColor(g_pCfgManager->getOptionValue("colors.ini", "mode")));
    }
}

void StatusWindow::onNickMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    // update the user's nickname if he's the one changing it
    QString oldNick = parseMsgPrefix(msg.m_prefix, MsgPrefixName);
    bool isMyNick = (oldNick.compare(m_pSharedSession->getNick(), Qt::CaseSensitive) == 0);

    QString textToPrint = QString("* %1 is now known as %2")
                .arg(oldNick)
                .arg(msg.m_params[0]);

    QColor nickColor(g_pCfgManager->getOptionValue("colors.ini", "nick"));
    if(isMyNick)
    {
        m_pSharedSession->setNick(msg.m_params[0]);
        printOutput(textToPrint, nickColor);
    }

    for(int i = 0; i < m_chanList.size(); ++i)
    {
        if(m_chanList[i]->hasUser(oldNick))
        {
            m_chanList[i]->changeUserNick(oldNick, msg.m_params[0]);
            m_chanList[i]->printOutput(textToPrint, nickColor);
        }
    }

    // will print a nick change message to the private message window
    // if we get a NICK message, which will only be if we're in
    // a channel with the person (or if the nick being changed is ours)
    for(int i = 0; i < m_privList.size(); ++i)
    {
        if(isMyNick)
        {
            m_privList[i]->printOutput(textToPrint, nickColor);
        }
        else
        {
            if(oldNick.compare(m_privList[i]->getTargetNick(), Qt::CaseInsensitive) == 0)
            {
                m_privList[i]->setTargetNick(msg.m_params[0]);
                m_privList[i]->printOutput(textToPrint, nickColor);
                break;
            }
        }
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

    QColor noticeColor(g_pCfgManager->getOptionValue("colors.ini", "notice"));
    printOutput(textToPrint, noticeColor);
}

void StatusWindow::onPartMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    ChannelWindow *pChanWin = dynamic_cast<ChannelWindow *>(getChildIrcWindow(msg.m_params[0]));
    if(!pChanWin)
    {
        // the window received a close message and sent
        // the PART message without expecting a reply,
        // so just ignore it
        return;
    }

    QString textToPrint;

    // if the PART message is of you leaving the channel
    if(m_pSharedSession->getNick().compare(parseMsgPrefix(msg.m_prefix, MsgPrefixName), Qt::CaseInsensitive) == 0)
    {
        textToPrint = QString("* You have left %1").arg(msg.m_params[0]);
        pChanWin->leaveChannel();
    }
    else
    {
        textToPrint = QString("* %1 (%2) has left %3")
                .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName))
                .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixUserAndHost))
                .arg(msg.m_params[0]);
        pChanWin->removeUser(parseMsgPrefix(msg.m_prefix, MsgPrefixName));
    }

    // if there's a part message
    QColor partColor(g_pCfgManager->getOptionValue("colors.ini", "part"));
    bool hasReason = (msg.m_paramsNum > 1 && !msg.m_params[1].isEmpty());
    if(hasReason)
    {
        textToPrint += QString(" (%1)").arg(msg.m_params[1]);
    }

    /*
    if(hasReason)
    {
        textToPrint += QString("</font><font color=%1>)</font>").arg(partColor.name());
    }
    */
    pChanWin->printOutput(textToPrint, partColor);
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
    CtcpRequestType requestType = getCtcpRequestType(msg);
    QString user = parseMsgPrefix(msg.m_prefix, MsgPrefixName);

    QString textToPrint;
    QColor color;

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
                    .arg(user)
                    .arg(action.mid(8, action.size()-9));
        }
        else		// regular CTCP requests
        {
            QString replyStr;
            QString requestTypeStr;

            switch(requestType)
            {
                case RequestTypeVersion:
                {
                    replyStr = "IMClient v0.00001";
                    requestTypeStr = "VERSION";
                    break;
                }
                case RequestTypeTime:
                {
                    replyStr = "time for you to get a watch";
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
                                                        .arg(user)
                                                        .arg(requestTypeStr)
                                                        .arg(replyStr);
                m_pSharedSession->sendData(textToSend);
            }

            color.setNamedColor(g_pCfgManager->getOptionValue("colors.ini", "ctcp"));
            textToPrint = QString("[CTCP %1 (from %2)]")
                                        .arg(requestTypeStr)
                                        .arg(user);
            printOutput(textToPrint, color);
            return;
        }
    }
    else	// not CTCP, so handle normally
    {
            color.setNamedColor(g_pCfgManager->getOptionValue("colors.ini", "say"));
            textToPrint = QString("<%1> %2")
                            .arg(user)
                            .arg(msg.m_params[1]);
    }

    // if the target is us, then it's an actual PM
    if(m_pSharedSession->getNick().compare(msg.m_params[0], Qt::CaseInsensitive) == 0)
    {
        QueryWindow *pQueryWin = dynamic_cast<QueryWindow *>(getChildIrcWindow(user));
        if(!pQueryWin)
        {
            // todo
            pQueryWin = new QueryWindow(m_pSharedSession, user);
            addQueryWindow(pQueryWin);
        }

        pQueryWin->printOutput(textToPrint, color);
    }
    else		// it's a channel
    {
        ChannelWindow *pChanWin = dynamic_cast<ChannelWindow *>(getChildIrcWindow(msg.m_params[0]));
        if(pChanWin)
        {
            // todo: figure out nick prefixes later
            pChanWin->printOutput(textToPrint, color);
        }
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

void StatusWindow::onTopicMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    OutputWindow *pChanWin = dynamic_cast<ChannelWindow *>(getChildIrcWindow(msg.m_params[0]));
    if(pChanWin)
    {
        QString textToPrint = QString("* %1 changes topic to: %2")
                    .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName))
                    .arg(msg.m_params[1]);
        QColor topicColor(g_pCfgManager->getOptionValue("colors.ini", "topic"));
        pChanWin->printOutput(textToPrint, topicColor);

        if(m_pManager)
        {
            QTreeWidgetItem *pItem = m_pManager->getItemFromWindow(pChanWin);
            QString titleWithTopic = QString("%1: %2")
                                .arg(pItem->text(0))
                                .arg(msg.m_params[1]);
            pChanWin->setTitle(stripCodes(titleWithTopic));
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
