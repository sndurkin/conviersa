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
#include "irc/Connection.h"

namespace irc {

// this class specifies various properties about a specific IRC server
// and provides an interface to them; it is maintained and used by an
// IrcStatusWindow, but used by the other connected IRC windows as well
class Session : public QObject, public QSharedData
{
    Q_OBJECT

private:
    Connection *m_pConn;

public:
    Session();

    // attaches the service to a specific server (essentially
    // reconstructs it
    void attachToServer(const QString &host, int port);

    // detaches itself from a server (it can no longer
    // be safely used)
    void detachFromServer();

    // returns whether it's attached or not
    bool isAttached() { return m_attached; }

    // sets the user's nickname
    void setNick(const QString &nick) { m_nick = nick; }

    // returns the user's nickname
    QString getNick() { return m_nick; }

    // sets the host's name
    void setHost(const QString &host) { m_host = host; }

    // returns the host's name
    QString getHost() { return m_host; }

    // returns the port number
    int getPort() { return m_port; }

    // sets the prefix rules
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

//signals:
//
//    // when you get any IRC message on the socket
//    void onPacketMessage();
//
//    // when a connection is made to the server
//    void onConnect();
//
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

protected:
    // tells whether the service is attached to a
    // specific server or not
    bool    m_attached;

    // stores the user's nickname
    QString m_nick;

    // stores the name of the host that we are connected to
    QString m_host;

    // stores the port number of the server we're connected to
    int m_port;

    // format: <mode1><prefix1><mode2><prefix2>[<mode3><prefix3>] ...
    // default value: o@v+
    QString m_prefixRules;

    // this comes from the 005 numeric, CHANMODES, which specifies
    // which channel modes the server supports, and which ones take
    // a parameter and which don't
    //
    // format: typeA,typeB,typeC,typeD
    QString m_chanModes;

    // this comes from the 005 numeric, MODES, which dictates
    // the maximum number of modes with parameters that may be set with
    // one message
    int     m_modeNum;
};

} // end namespace
