/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QObject>
#include <QString>
#include <QSharedData>
#include <QTime>
#include "cv/Connection.h"
#include "cv/Parser.h"
#include "cv/EventManager.h"

namespace cv {

class MessageEvent : public Event
{
    Message m_msg;

public:
    MessageEvent(const Message &msg)
      : m_msg(msg)
    { }

    Message getMessage() { return m_msg; }
};

class DataEvent : public Event
{
    QString m_data;

public:
    DataEvent(const QString &data)
      : m_data(data)
    { }

    QString getData() { return m_data; }
};

// this class provides the entire interface to an IRC server
class Session : public QObject, public QSharedData
{
    Q_OBJECT

public:
    Session(const QString& nick);
    ~Session();

    void connectToServer(const QString &host, int port);
    void disconnectFromServer();
    bool isConnected() { return m_pConn->isConnected(); }

    // public functions for sending messages
    void sendData(const QString &data);
    void sendPrivmsg(const QString &target, const QString &msg);
    void sendAction(const QString &target, const QString &msg);

    void setHost(const QString &host) { m_host = host; }
    QString getHost() { return m_host; }
    int getPort() { return m_port; }
    void setNick(const QString &nick) { m_nick = nick; }
    QString getNick() { return m_nick; }
    bool isMyNick(const QString &nick) { return (m_nick.compare(nick, Qt::CaseSensitive) == 0); }

    EventManager *getEventManager() { return m_pEvtMgr; }

    // sets the prefix rules supported by the server
    void setPrefixRules(const QString &prefixRules);

    // sets the channel modes supported by the server
    void setChanModes(const QString &chanModes);

    // returns the string of channel modes
    QString getChanModes() { return m_chanModes; }

    // sets the mode num
    void setModeNum(int modeNum) { m_modeNum = modeNum; }

    // retrieves the mode num
    int getModeNum() { return m_modeNum; }

    // compares two prefix characters in a nickname as per
    // the server's specification
    //
    // returns -1 if prefix1 is less in value than prefix2
    // returns 0 if prefix1 is equal to prefix2
    // returns 1 if prefix1 is greater in value than prefix2
    int compareNickPrefixes(const QChar &prefix1, const QChar &prefix2);

    // returns the corresponding prefix rule to the character
    // provided by match
    //
    // match can either be a nick prefix or the corresponding mode
    QChar getPrefixRule(const QChar &match);

    // returns true if the character is a set prefix for the server,
    // returns false otherwise
    //
    // example prefixes: @, %, +
    bool isNickPrefix(const QChar &prefix);

    // handles the preliminary processing for all messages;
    // this will fire events for specific message types, and store
    // some information as a result of others (like numerics)
    void processMessage(const Message &msg);

//    // when you are identified on the server
//    void onIdentify();
//
//    // when you get a private message
//    void onPrivateMessage();
//
//    // when you join a channel on the server
//    void onChannelJoin();
//
//    // when any channel event occurs
//    //void onChannelEvent();
//
//    // when a CTCP request is received
//    void onCTCPRequest();
//
//    // when a DCC request is received
//    void onDCCRequest();

public slots:
    // these are connected to the Connection class and are called when
    // Connection emits them
    void onConnect();
    void onDisconnect();
    void onReceiveData(const QString &data);

private:
    // the actual connection to the server
    Connection *    m_pConn;

    // event manager for the session object
    EventManager *  m_pEvtMgr;

    // stores the user's nickname
    QString         m_nick;

    // stores the name of the host that we are connected to
    QString         m_host;

    // stores the port number of the server we're connected to
    int             m_port;

    // format: <mode1><prefix1><mode2><prefix2>[<mode3><prefix3>] ...
    // default value: o@v+
    QString         m_prefixRules;

    // this comes from the 005 numeric, CHANMODES, which specifies
    // which channel modes the server supports, and which ones take
    // a parameter and which don't
    //
    // format: typeA,typeB,typeC,typeD
    QString         m_chanModes;

    // this comes from the 005 numeric, MODES, which dictates
    // the maximum number of modes with parameters that may be set with
    // one message
    int             m_modeNum;

    // this acts as a more persistent buffer for receiving data from
    // the Connection object, so that it can be separated by '\n' and
    // then parsed
    QString         m_prevData;
};

} // end namespace
