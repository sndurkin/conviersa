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
#include "irc/Session.h"
#include "cv/qext.h"
#include "cv/WindowManager.h"
#include "cv/ConfigManager.h"
#include "cv/irc/types.h"
#include "cv/irc/StatusWindow.h"
#include "cv/irc/ChannelListWindow.h"
#include "cv/irc/ChannelWindow.h"
#include "cv/irc/QueryWindow.h"
#include "cv/irc/OutputWindowScrollBar.h"

#define DEBUG_MESSAGES 0

namespace cv { namespace irc {

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

    m_pSharedSession = new Session;
    m_pSharedConn = new Connection(this, m_pCodec);

    QObject::connect(m_pSharedConn.data(), SIGNAL(disconnected()), this, SLOT(handleDisconnect()));
}

StatusWindow::~StatusWindow()
{
    m_pSharedConn->disconnect();
}

int StatusWindow::getIrcWindowType()
{
    return IRC_STATUS_WIN_TYPE;
}

// handles the data received from the Connection class
void StatusWindow::handleData(QString &data)
{
#if DEBUG_MESSAGES
    QString blah = data;
    blah.remove(blah.size()-2,2);
    printDebug(blah);
#endif

    Message msg = parseData(data);

    if(msg.m_isNumeric)
    {
        switch(msg.m_command)
        {
            case 1:
            {
                handle001Numeric(msg);
                printOutput(getNumericText(msg));
                break;
            }
            case 2:
            {
                handle002Numeric(msg);
                printOutput(getNumericText(msg));
                break;
            }
            case 5:
            {
                handle005Numeric(msg);
                printOutput(getNumericText(msg));
                break;
            }
            // RPL_AWAY
            case 301:
            {
                handle301Numeric(msg);
                break;
            }
            /*// RPL_USERHOST
            case 302:
            {

                break;
            }
            // RPL_isON
            case 303:
            {

                break;
            }*/
            // RPL_WHOisIDLE
            case 317:
            {
                handle317Numeric(msg);
                break;
            }
            // RPL_LisTSTART
            case 321:
            {
                handle321Numeric(msg);
                break;
            }
            // RPL_LisT
            case 322:
            {
                handle322Numeric(msg);
                break;
            }
            // RPL_LisTEND
            case 323:
            {
                handle323Numeric(msg);
                break;
            }
            // RPL_WHOisACCOUNT
            case 330:
            {
                handle330Numeric(msg);
                break;
            }
            // RPL_TOPIC
            case 332:
            {
                handle332Numeric(msg);
                break;
            }
            // states when topic was last set
            case 333:
            {
                handle333Numeric(msg);
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
            // ERR_NOSUCKNICK
            case 401:
            {
                handle401Numeric(msg);
                break;
            }
            // ERR_CANNOTSENDTOCHAN
            case 404:
            {
                // handles 404 numeric messages too
                handle401Numeric(msg);
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
    else
    {
        switch(msg.m_command)
        {
            case IRC_COMMAND_ERROR:
            {
                // prints the error message
                printOutput(msg.m_params[0]);
                break;
            }
            case IRC_COMMAND_INVITE:
            {
                handleInviteMsg(msg);
                break;
            }
            case IRC_COMMAND_JOIN:
            {
                handleJoinMsg(msg);
                break;
            }
            case IRC_COMMAND_KICK:
            {
                handleKickMsg(msg);
                break;
            }
            case IRC_COMMAND_MODE:
            {
                handleModeMsg(msg);
                break;
            }
            case IRC_COMMAND_NICK:
            {
                handleNickMsg(msg);
                break;
            }
            case IRC_COMMAND_NOTICE:
            {
                handleNoticeMsg(msg);
                break;
            }
            case IRC_COMMAND_PART:
            {
                handlePartMsg(msg);
                break;
            }
            case IRC_COMMAND_PING:
            {
                m_pSharedConn->send("PONG :" + msg.m_params[0]);
                break;
            }
            case IRC_COMMAND_PONG:
            {
                handlePongMsg(msg);
                break;
            }
            case IRC_COMMAND_PRIVMSG:
            {
                handlePrivMsg(msg);
                break;
            }
            case IRC_COMMAND_QUIT:
            {
                handleQuitMsg(msg);
                break;
            }
            case IRC_COMMAND_TOPIC:
            {
                handleTopicMsg(msg);
                break;
            }
            case IRC_COMMAND_WALLOPS:
            {
                handleWallopsMsg(msg);
                break;
            }
            default:
            {
                // print the whole raw line
                printOutput(data);
            }
        }
    }
}

// returns a pointer to the IIrcWindow if it exists
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
void StatusWindow::addChanWindow(ChannelWindow *pChanWin)
{
    if(m_pManager)
        m_pManager->addWindow(pChanWin, m_pManager->getItemFromWindow(this));
    m_chanList.append(pChanWin);
    QObject::connect(pChanWin, SIGNAL(chanWindowClosing(ChannelWindow *)),
                this, SLOT(removeChanWindow(ChannelWindow *)));
}

// removes a channel from the list
void StatusWindow::removeChanWindow(ChannelWindow *pChanWin)
{
    m_chanList.removeOne(pChanWin);
}

// adds a PM window to the list
void StatusWindow::addPrivWindow(QueryWindow *pPrivWin)
{
    if(m_pManager)
        m_pManager->addWindow(pPrivWin, m_pManager->getItemFromWindow(this));
    m_privList.append(pPrivWin);
    QObject::connect(pPrivWin, SIGNAL(privWindowClosing(QueryWindow *)),
                this, SLOT(removePrivWindow(QueryWindow *)));
}

// removes a PM window from the list
void StatusWindow::removePrivWindow(QueryWindow *pPrivWin)
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

void StatusWindow::handle001Numeric(const Message &msg)
{
    // msg.m_params[0]: my nick
    // msg.m_params[1]: "Welcome to the <server name> IRC Network, <nick>[!user@host]"
    //
    // check to make sure nickname hasn't changed; some or all servers apparently don't
    // send you a NICK message when your nickname conflicts with another user upon
    // first entering the server, and you try to change it
    if(m_pSharedSession->getNick().compare(msg.m_params[0], Qt::CaseInsensitive) != 0)
    {
        m_pSharedSession->setNick(msg.m_params[0]);
    }

    QString header = "Welcome to the ";
    if(msg.m_params[1].startsWith(header, Qt::CaseInsensitive))
    {
        int idx = msg.m_params[1].indexOf(' ', header.size(), Qt::CaseInsensitive);
        if(idx >= 0)
        {
            // change name of the window to the name of the network
            QString networkName = msg.m_params[1].mid(header.size(), idx - header.size());
            setTitle(networkName);
            setWindowName(networkName);
        }
    }
}

void StatusWindow::handle002Numeric(const Message &msg)
{
    // msg.m_params[0]: my nick
    // msg.m_params[1]: "Your host is ..."
    QString header = "Your host is ";
    QString hostStr = msg.m_params[1].section(',', 0, 0);
    if(hostStr.startsWith(header))
    {
        m_pSharedSession->setHost(hostStr.mid(header.size()));
    }
}

void StatusWindow::handle005Numeric(const Message &msg)
{
    // we only go to the second-to-last parameter,
    // because the last parameter holds "are supported
    // by this server"
    for(int i = 1; i < msg.m_paramsNum-1; ++i)
    {
        if(msg.m_params[i].startsWith("PREFIX=", Qt::CaseInsensitive))
        {
            m_pSharedSession->setPrefixRules(getPrefixRules(msg.m_params[i]));
        }
        else if(msg.m_params[i].compare("NAMESX", Qt::CaseInsensitive) == 0)
        {
            // lets the server know we support multiple nick prefixes
            //
            // todo: UHNAMES?
            m_pSharedConn->send("PROTOCTL NAMESX");
        }
        else if(msg.m_params[i].startsWith("CHANMODES=", Qt::CaseInsensitive))
        {
            m_pSharedSession->setChanModes(msg.m_params[i].section('=', 1));
        }
    }
}

void StatusWindow::handle301Numeric(const Message &msg)
{
    // msg.m_params[0]: my nick
    // msg.m_params[1]: nick
    // msg.m_params[2]: away message
        QString textToPrint = QString("%1 is away: %2")
                                .arg(msg.m_params[1])
                                .arg(convertDataToHtml(msg.m_params[2]));
    printOutput(textToPrint);
}

void StatusWindow::handle317Numeric(const Message &msg)
{
    // msg.m_params[0]: my nick
    // msg.m_params[1]: nick
    // msg.m_params[2]: seconds
    // two options here:
        //	1)      msg.m_params[3]: "seconds idle"
        //	2)      msg.m_params[3]: unix time
    //		msg.m_params[4]: "seconds idle, signon time"

    // get the number of idle seconds first, convert
    // to h, m, s format
    bool conversionOk;
    uint numSecs = msg.m_params[2].toInt(&conversionOk);
    if(conversionOk)
    {
        QString textToPrint = QString("%1 has been idle ").arg(msg.m_params[1]);

        // 24 * 60 * 60 = 86400
        uint numDays = numSecs / 86400;
        if(numDays)
        {
            textToPrint += QString::number(numDays);
            if(numDays == 1)
            {
                textToPrint += "day ";
            }
            else
            {
                textToPrint += "days ";
            }
            numSecs = numSecs % 86400;
        }

        // 60 * 60 = 3600
        uint numHours = numSecs / 3600;
        if(numHours)
        {
            textToPrint += QString::number(numHours);
            if(numHours == 1)
            {
                textToPrint += "hr ";
            }
            else
            {
                textToPrint += "hrs ";
            }
            numSecs = numSecs % 3600;
        }

        uint numMinutes = numSecs / 60;
        if(numMinutes)
        {
            textToPrint += QString::number(numMinutes);
            if(numMinutes == 1)
            {
                textToPrint += "min ";
            }
            else
            {
                textToPrint += "mins ";
            }
            numSecs = numSecs % 60;
        }

        if(numSecs)
        {
            textToPrint += QString::number(numSecs);
            if(numSecs == 1)
            {
                textToPrint += "sec ";
            }
            else
            {
                textToPrint += "secs ";
            }
        }

        // remove trailing space
        textToPrint.remove(textToPrint.size()-1, 1);

        // right now this will only support 5 parameters
        // (1 extra for the signon time), but i can easily
        // add support for more later
        if(msg.m_paramsNum > 4)
        {
            uint uTime = msg.m_params[3].toInt(&conversionOk);
            if(conversionOk)
            {
                QDateTime dt;
                dt.setTime_t(uTime);

                QString strDate = dt.toString("ddd MMM dd");
                QString strTime = dt.toString("hh:mm:ss");

                textToPrint += QString(", signed on %1 %2").arg(strDate).arg(strTime);
            }
        }

        printOutput(textToPrint);
    }
}

void StatusWindow::handle321Numeric(const Message &msg)
{
    if(!m_pChanListWin)
    {
        m_pChanListWin = new (std::nothrow) ChannelListWindow(m_pSharedConn);
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
            m_pSharedConn->send("LisT STOP");
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

void StatusWindow::handle330Numeric(const Message &msg)
{
    // msg.m_params[0]: my nick
    // msg.m_params[1]: nick
    // msg.m_params[2]: login/auth
    // msg.m_params[3]: "is logged in as"
    QString textToPrint = QString("%1 %2: %3").arg(msg.m_params[1])
                                .arg(msg.m_params[3])
                                .arg(msg.m_params[2]);
    printOutput(textToPrint);
}

void StatusWindow::handle332Numeric(const Message &msg)
{
    // msg.m_params[0]: my nick
    // msg.m_params[1]: channel
    // msg.m_params[2]: topic
    OutputWindow *pChanWin = getChildIrcWindow(msg.m_params[1]);
    if(pChanWin)
    {
        QString titleWithTopic = QString("%1: %2").arg(pChanWin->getWindowName())
                                    .arg(stripCodes(msg.m_params[2]));
        pChanWin->setTitle(titleWithTopic);

        QColor topicColor(g_pCfgManager->getOptionValue("colors.ini", "topic"));
        QString textToPrint = QString("* Topic is: %1").arg(msg.m_params[2]);
        pChanWin->printOutput(textToPrint, topicColor);
    }
    else
    {
        printOutput(getNumericText(msg));
    }
}

void StatusWindow::handle333Numeric(const Message &msg)
{
    // msg.m_params[0]: my nick
    // msg.m_params[1]: channel
    // msg.m_params[2]: nick
    // msg.m_params[3]: unix time
    OutputWindow *pChanWin = getChildIrcWindow(msg.m_params[1]);

    bool conversionOk;
    uint uTime = msg.m_params[3].toInt(&conversionOk);
    if(conversionOk)
    {
        QDateTime dt;
        dt.setTime_t(uTime);
        QString strDate = dt.toString("ddd MMM dd");
        QString strTime = dt.toString("hh:mm:ss");

        QString textToPrint;
        if(pChanWin)
        {
            textToPrint += "* Topic set by ";
        }
        else
        {
            textToPrint += QString("%1 topic set by ").arg(msg.m_params[1]);
        }

        textToPrint += QString("%1 on %2 %3").arg(msg.m_params[2]).arg(strDate).arg(strTime);

        if(pChanWin)
        {
            pChanWin->printOutput(textToPrint, QColor(g_pCfgManager->getOptionValue("colors.ini", "topic")));
        }
        else
        {
            printOutput(textToPrint);
        }
    }
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

// covers both 401 and 404 numerics
void StatusWindow::handle401Numeric(const Message &msg)
{
    // msg.m_params[0]: my nick
    // msg.m_params[1]: nick/channel
    // msg.m_params[2]: "No such nick/channel"
    OutputWindow *pPrivWin = getChildIrcWindow(msg.m_params[1]);
    if(pPrivWin)
    {
        pPrivWin->printOutput(getNumericText(msg));
    }
    else
    {
        printOutput(getNumericText(msg));
    }
}

void StatusWindow::handleInviteMsg(const Message &msg)
{
    QString textToPrint = QString("* %1 has invited you to %2")
                .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName))
                .arg(msg.m_params[1]);
    printOutput(textToPrint, QColor(g_pCfgManager->getOptionValue("colors.ini", "invite")));
}

void StatusWindow::handleJoinMsg(const Message &msg)
{
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
            pChanWin = new (std::nothrow) ChannelWindow(m_pSharedSession, m_pSharedConn, msg.m_params[0]);
            if(!pChanWin)
            {
                printError("Allocation of a new channel window failed.");
                return;
            }
            addChanWindow(pChanWin);
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

void StatusWindow::handleKickMsg(const Message &msg)
{
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

void StatusWindow::handleModeMsg(const Message &msg)
{
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

void StatusWindow::handleNickMsg(const Message &msg)
{
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

void StatusWindow::handleNoticeMsg(const Message &msg)
{
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

void StatusWindow::handlePartMsg(const Message &msg)
{
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

void StatusWindow::handlePongMsg(const Message &msg)
{
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

void StatusWindow::handlePrivMsg(const Message &msg)
{
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
                m_pSharedConn->send(textToSend);
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
        QueryWindow *pPrivWin = dynamic_cast<QueryWindow *>(getChildIrcWindow(user));
        if(!pPrivWin)
        {
            // todo
            pPrivWin = new QueryWindow(m_pSharedSession, m_pSharedConn, user);
            addPrivWindow(pPrivWin);
        }

        pPrivWin->printOutput(textToPrint, color);
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

void StatusWindow::handleQuitMsg(const Message &msg)
{
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

void StatusWindow::handleTopicMsg(const Message &msg)
{
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

void StatusWindow::handleWallopsMsg(const Message &msg)
{
    QString textToPrint = QString("* WALLOPS from %1: %2")
                .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName))
                .arg(msg.m_params[0]);
    printOutput(textToPrint);
}

// handles a disconnection fired from the Connection object
void StatusWindow::handleDisconnect()
{
    printOutput("* Disconnected.");
    setTitle("Server Window");
    setWindowName("Server Window");
}

} } // end namespaces
