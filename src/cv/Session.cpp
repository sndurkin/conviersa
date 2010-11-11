/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QCoreApplication>
#include "cv/Session.h"
#include "cv/Parser.h"

namespace cv {

Session::Session(const QString& nick)
  : m_nick(nick),
    m_prevData("")
{
    m_pConn = new Connection;
    QObject::connect(m_pConn, SIGNAL(connected()), this, SLOT(onConnect()));
    QObject::connect(m_pConn, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    QObject::connect(m_pConn, SIGNAL(dataReceived(QString)), this, SLOT(onReceiveData(QString)));

    m_pEvtMgr = new EventManager;
    m_pEvtMgr->createEvent("onConnect");
    m_pEvtMgr->createEvent("onDisconnect");
    m_pEvtMgr->createEvent("onReceiveData");
    m_pEvtMgr->createEvent("onErrorMessage");
    m_pEvtMgr->createEvent("onInviteMessage");
    m_pEvtMgr->createEvent("onJoinMessage");
    m_pEvtMgr->createEvent("onKickMessage");
    m_pEvtMgr->createEvent("onModeMessage");
    m_pEvtMgr->createEvent("onNickMessage");
    m_pEvtMgr->createEvent("onNoticeMessage");
    m_pEvtMgr->createEvent("onPartMessage");
    m_pEvtMgr->createEvent("onPongMessage");
    m_pEvtMgr->createEvent("onPrivmsgMessage");
    m_pEvtMgr->createEvent("onQuitMessage");
    m_pEvtMgr->createEvent("onTopicMessage");
    m_pEvtMgr->createEvent("onWallopsMessage");
    m_pEvtMgr->createEvent("onNumericMessage");
    m_pEvtMgr->createEvent("onUnknownMessage");
}

Session::~Session()
{
    delete m_pConn;
    delete m_pEvtMgr;
}

void Session::connectToServer(const QString& host, int port)
{
    connectToServer(host, port, "Ronnie Reagan", "conviersa");
}

void Session::connectToServer(const QString &host, int port, const QString &name, const QString &nick)
{
    m_name = name;
    m_nick = nick;
    m_host = host;
    m_port = port;

    // this defaults the server's prefix rules (for users in a channel)
    // to @ for +o (oper), and + for +v (voice); this is just in case
    // a server does not send explicit prefix rules to the Session
    //
    // for more info see the definition in Session.h
    m_prefixRules = "o@v+";

    m_pConn->connectToHost(host.toAscii(), port);
}

void Session::disconnectFromServer()
{
    m_pConn->disconnectFromHost();
}

void Session::sendData(const QString &data)
{
    m_pConn->send(data + "\r\n");
}

void Session::sendPrivmsg(const QString &target, const QString &msg)
{
    m_pConn->send("PRIVMSG " + target + " :" + msg + "\r\n");
}

void Session::sendAction(const QString &target, const QString &msg)
{
    m_pConn->send("PRIVMSG " + target + " :\1ACTION " + msg + "\1\r\n");
}

// sets the prefix rules supported by the server
void Session::setPrefixRules(const QString &prefixRules)
{
    m_prefixRules = prefixRules;

    // stick these modes in with the type B channel modes
    if(!m_chanModes.isEmpty())
    {
        int index = m_chanModes.indexOf(',');
        if(index >= 0)
        {
            for(int i = 0; i < m_prefixRules.size(); i += 2)
                m_chanModes.insert(index+1, m_prefixRules[i]);
        }
    }
}

// sets the channel modes supported by the server
void Session::setChanModes(const QString &chanModes)
{
    m_chanModes = chanModes;

    // stick these modes in with the type B channel modes
    //
    // we do it in both functions to ensure the right ones are added
    int index = m_chanModes.indexOf(',');
    if(index >= 0)
    {
        for(int i = 0; i < m_prefixRules.size(); i += 2)
            m_chanModes.insert(index+1, m_prefixRules[i]);
    }
}

// compares two prefix characters in a nickname as per
// the server's specification
//
// returns -1 if prefix1 is less in value than prefix2
// returns 0 if prefix1 is equal to prefix2
// returns 1 if prefix1 is greater in value than prefix2
int Session::compareNickPrefixes(const QChar &prefix1, const QChar &prefix2)
{
    if(prefix1 == prefix2)
        return 0;

    for(int i = 1; i < m_prefixRules.size(); i += 2)
    {
        if(m_prefixRules[i] == prefix1)
            return -1;

        if(m_prefixRules[i] == prefix2)
            return 1;
    }

    return 0;
}

// returns the corresponding prefix rule to the character
// provided by match
//
// match can either be a nick prefix or the corresponding mode
QChar Session::getPrefixRule(const QChar &match)
{
    for(int i = 0; i < m_prefixRules.size(); ++i)
    {
        if(m_prefixRules[i] == match)
        {
            // if it's even, the corresponding letter is at i+1
            if(i % 2 == 0)
                return m_prefixRules[i+1];
            // otherwise, the corresponding letter is at i-1
            else
                return m_prefixRules[i-1];
        }
    }

    return '\0';
}

// returns true if the character is a set prefix for the server,
// returns false otherwise
//
// example prefixes: @, %, +
bool Session::isNickPrefix(const QChar &prefix)
{
    // the first prefix begins at index 1
    for(int i = 1; i < m_prefixRules.size(); i += 2)
    {
        if(m_prefixRules[i] == prefix)
            return true;
    }

    return false;
}

// handles the preliminary processing for all messages;
// this will fire events for specific message types, and store
// some information as a result of others (like numerics)
void Session::processMessage(const Message &msg)
{
    Event *evt = new MessageEvent(msg);
    if(msg.m_isNumeric)
    {
        switch(msg.m_command)
        {
            case 1:
            {
                // check to make sure nickname hasn't changed; some or all servers apparently don't
                // send you a NICK message when your nickname conflicts with another user upon
                // first entering the server, and you try to change it
                if(!isMyNick(msg.m_params[0]))
                {
                    setNick(msg.m_params[0]);
                }

                break;
            }
            case 2:
            {
                // msg.m_params[0]: my nick
                // msg.m_params[1]: "Your host is ..."
                QString header = "Your host is ";
                QString hostStr = msg.m_params[1].section(',', 0, 0);
                if(hostStr.startsWith(header))
                {
                    setHost(hostStr.mid(header.size()));
                }

                break;
            }
            case 5:
            {
                // we only go to the second-to-last parameter,
                // because the last parameter holds "are supported
                // by this server"
                for(int i = 1; i < msg.m_paramsNum-1; ++i)
                {
                    if(msg.m_params[i].startsWith("PREFIX=", Qt::CaseInsensitive))
                    {
                        setPrefixRules(getPrefixRules(msg.m_params[i]));
                    }
                    else if(msg.m_params[i].compare("NAMESX", Qt::CaseInsensitive) == 0)
                    {
                        // lets the server know we support multiple nick prefixes
                        //
                        // todo: UHNAMES?
                        sendData("PROTOCTL NAMESX");
                    }
                    else if(msg.m_params[i].startsWith("CHANMODES=", Qt::CaseInsensitive))
                    {
                        setChanModes(msg.m_params[i].section('=', 1));
                    }
                }

                break;
            }
        }

        m_pEvtMgr->fireEvent("onNumericMessage", evt);
    }
    else
    {
        switch(msg.m_command)
        {
            case IRC_COMMAND_ERROR:
            {
                m_pEvtMgr->fireEvent("onErrorMessage", evt);
                break;
            }
            case IRC_COMMAND_INVITE:
            {
                m_pEvtMgr->fireEvent("onInviteMessage", evt);
                break;
            }
            case IRC_COMMAND_JOIN:
            {
                m_pEvtMgr->fireEvent("onJoinMessage", evt);
                break;
            }
            case IRC_COMMAND_KICK:
            {
                m_pEvtMgr->fireEvent("onKickMessage", evt);
                break;
            }
            case IRC_COMMAND_MODE:
            {
                m_pEvtMgr->fireEvent("onModeMessage", evt);
                break;
            }
            case IRC_COMMAND_NICK:
            {
                m_pEvtMgr->fireEvent("onNickMessage", evt);

                // update the user's nickname if he's the one changing it
                QString oldNick = parseMsgPrefix(msg.m_prefix, MsgPrefixName);
                if(isMyNick(oldNick))
                {
                    setNick(msg.m_params[0]);
                }
                break;
            }
            case IRC_COMMAND_NOTICE:
            {
                m_pEvtMgr->fireEvent("onNoticeMessage", evt);
                break;
            }
            case IRC_COMMAND_PART:
            {
                m_pEvtMgr->fireEvent("onPartMessage", evt);
                break;
            }
            case IRC_COMMAND_PING:
            {
                sendData("PONG :" + msg.m_params[0]);
                break;
            }
            case IRC_COMMAND_PONG:
            {
                m_pEvtMgr->fireEvent("onPongMessage", evt);
                break;
            }
            case IRC_COMMAND_PRIVMSG:
            {
                m_pEvtMgr->fireEvent("onPrivmsgMessage", evt);
                break;
            }
            case IRC_COMMAND_QUIT:
            {
                m_pEvtMgr->fireEvent("onQuitMessage", evt);
                break;
            }
            case IRC_COMMAND_TOPIC:
            {
                m_pEvtMgr->fireEvent("onTopicMessage", evt);
                break;
            }
            case IRC_COMMAND_WALLOPS:
            {
                m_pEvtMgr->fireEvent("onWallopsMessage", evt);
                break;
            }
            default:
            {
                m_pEvtMgr->fireEvent("onUnknownMessage", evt);
            }
        }
    }

    delete evt;
}

void Session::onConnect()
{
    // todo: use options
    QString info = "PASS hello\r\nNICK " + m_nick + "\r\nUSER " + m_nick + " tolmoon tolsun :" + m_name;
    sendData(info);

    // todo: find out what to do with Event *
    m_pEvtMgr->fireEvent("onConnect", NULL);
}

void Session::onDisconnect()
{
    // todo: find out what to do with Event *
    m_pEvtMgr->fireEvent("onDisconnect", NULL);
}

void Session::onReceiveData(const QString &data)
{
    m_prevData += data;

    // check for a message within the data buffer, to ensure only
    // whole messages are handled
    int numChars;
    while((numChars = m_prevData.indexOf('\n') + 1) > 0)
    {
        // retrieves the entire message up to and including the terminating '\n' character
        QString msgData = m_prevData.left(numChars);
        m_prevData.remove(0, numChars);

        Event *evt = new DataEvent(msgData);
        m_pEvtMgr->fireEvent("onReceiveData", evt);
        delete evt;

        Message msg = parseData(msgData);
        processMessage(msg);
        QCoreApplication::processEvents(QEventLoop::ExcludeSocketNotifiers, 5);
    }
}

} // end namespace
