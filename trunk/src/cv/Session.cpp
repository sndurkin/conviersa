/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include "cv/Session.h"
#include "cv/Parser.h"

namespace cv {

Session::Session(const QString& nick)
  : QSharedData(),
    m_nick(nick),
    m_prevData("")
{
    m_pConn = new Connection;
    QObject::connect(m_pConn, SIGNAL(connected()), this, SLOT(onConnect()));
    QObject::connect(m_pConn, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    QObject::connect(m_pConn, SIGNAL(dataReceived(QString)), this, SIGNAL(dataReceived(QString)));
    QObject::connect(m_pConn, SIGNAL(dataReceived(QString)), this, SLOT(onReceiveData(QString)));
}

Session::~Session()
{
    delete m_pConn;
}

void Session::connectToServer(const QString& host, int port = 6667)
{
    m_host = host;
    m_port = port;

    // this defaults the server's prefix rules (for users in a channel)
    // to @ for +o (oper), and + for +v (voice); this is just in case
    // a server does not send explicit prefix rules to the Session
    //
    // for more info see the definition in Session.h
    m_prefixRules = "o@v+"; // TODO: wtf is this

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
// this will emit signals for specific message types, and store
// some information as a result of others (like numerics)
void Session::processMessage(const Message &msg)
{
    if(msg.m_isNumeric)
    {
        emit numericMessage(msg);
    }
    else
    {
        switch(msg.m_command)
        {
            case IRC_COMMAND_ERROR:
                emit errorMessage(msg);
                break;
            case IRC_COMMAND_INVITE:
                emit inviteMessage(msg);
                break;
            case IRC_COMMAND_JOIN:
                emit joinMessage(msg);
                break;
            case IRC_COMMAND_KICK:
                emit kickMessage(msg);
                break;
            case IRC_COMMAND_MODE:
                emit modeMessage(msg);
                break;
            case IRC_COMMAND_NICK:
                emit nickMessage(msg);
                break;
            case IRC_COMMAND_NOTICE:
                emit noticeMessage(msg);
                break;
            case IRC_COMMAND_PART:
                emit partMessage(msg);
                break;
            case IRC_COMMAND_PING:
                sendData("PONG :" + msg.m_params[0]);
                break;
            case IRC_COMMAND_PONG:
                emit pongMessage(msg);
                break;
            case IRC_COMMAND_PRIVMSG:
                emit privmsgMessage(msg);
                break;
            case IRC_COMMAND_QUIT:
                emit quitMessage(msg);
                break;
            case IRC_COMMAND_TOPIC:
                emit topicMessage(msg);
                break;
            case IRC_COMMAND_WALLOPS:
                emit wallopsMessage(msg);
                break;
            default:
                emit dataParsed(msg);
        }
    }
}

void Session::onConnect()
{
    // todo: use options
    QString info = "PASS hello\r\nNICK " + m_nick + "\r\nUSER guest tolmoon tolsun :Ronnie Reagan";
    sendData(info);

    emit connected();
}

void Session::onDisconnect()
{
    emit disconnected();
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
        Message msg = parseData(msgData);
        processMessage(msg);
    }
}

} // end namespace