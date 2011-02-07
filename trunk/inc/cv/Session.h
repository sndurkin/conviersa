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

class ConnectionEvent : public Event
{
    QString *   m_pHost;
    quint16     m_port;

    // only relevant for the connectFailed event
    QAbstractSocket::SocketError m_error;

public:
    ConnectionEvent(QString &host, quint16 port)
      : m_pHost(&host),
        m_port(port)
    { }
    ConnectionEvent(QString &host, quint16 port, QAbstractSocket::SocketError error)
      : m_pHost(&host),
        m_port(port),
        m_error(error)
    { }

    QString getHost() { return *m_pHost; }
    quint16 getPort() { return m_port; }
    QAbstractSocket::SocketError error() { return m_error; }
};

//-----------------------------------//

class MessageEvent : public Event
{
    Message m_msg;

public:
    MessageEvent(const Message &msg)
      : m_msg(msg)
    { }

    Message getMessage() { return m_msg; }
};

//-----------------------------------//

class DataEvent : public Event
{
    QString m_data;

public:
    DataEvent(const QString &data)
      : m_data(data)
    { }

    QString getData() { return m_data; }
};

//-----------------------------------//

// this class provides the entire interface to an IRC server
class Session : public QObject, public QSharedData
{
    Q_OBJECT

private:
    // the actual connection to the server
    ThreadedConnection *   m_pConn;

    // stores the user's nickname
    QString             m_nick;

    // stores the name of the host that we are connected to
    QString             m_host;

    // stores the port number of the server we're connected to
    int                 m_port;

    // stores the user's name (used for USER message)
    QString             m_name;

    // format: <mode1><prefix1><mode2><prefix2>[<mode3><prefix3>] ...
    // default value: o@v+
    QString             m_prefixRules;

    // this comes from the 005 numeric, CHANMODES, which specifies
    // which channel modes the server supports, and which ones take
    // a parameter and which don't
    //
    // format: typeA,typeB,typeC,typeD
    QString             m_chanModes;

    // this comes from the 005 numeric, MODES, which dictates
    // the maximum number of modes with parameters that may be set with
    // one message
    int                 m_modeNum;

    // this acts as a more persistent buffer for receiving data from
    // the Connection object, so that it can be separated by '\n' and
    // then parsed
    QString             m_prevData;

public:
    Session(const QString& nick);
    ~Session();

    void connectToServer(const QString &host, int port);
    void connectToServer(const QString &host, int port, const QString &name, const QString &nick);
    void disconnectFromServer();
    bool isConnected() { return m_pConn->isConnected(); }

    // public functions for sending messages
    void sendData(const QString &data);
    void onSendData(Event *pEvt);
    void sendPrivmsg(const QString &target, const QString &msg);
    void sendAction(const QString &target, const QString &msg);

    void setHost(const QString &host) { m_host = host; }
    QString getHost() { return m_host; }
    int getPort() { return m_port; }
    void setNick(const QString &nick) { m_nick = nick; }
    QString getNick() { return m_nick; }
    bool isMyNick(const QString &nick) { return (m_nick.compare(nick, Qt::CaseSensitive) == 0); }

    void setPrefixRules(const QString &prefixRules);
    void setChanModes(const QString &chanModes);
    QString getChanModes() { return m_chanModes; }
    void setModeNum(int modeNum) { m_modeNum = modeNum; }
    int getModeNum() { return m_modeNum; }

    int compareNickPrefixes(const QChar &prefix1, const QChar &prefix2);
    QChar getPrefixRule(const QChar &match);
    bool isNickPrefix(const QChar &prefix);

    void processMessage(const Message &msg);

signals:
    void connectToHost(QString, quint16);

public slots:
    // these are connected to the Connection class and are called when
    // Connection emits them
    void onConnecting();
    void onConnect();
    void onFailedConnect();
    void onDisconnect();
    void onReceiveData(const QString &data);
};

} // end namespace
