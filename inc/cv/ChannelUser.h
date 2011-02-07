/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QString>

namespace cv {

class Session;

class ChannelUser
{
    Session *   m_pSession;

    // msg prefix: nick!user@host
    QString     m_nickname;
    QString     m_prefixes;
    QString     m_user;
    QString     m_host;

    // priority of the nickname for tab-completion
    int         m_priority;

public:
    ChannelUser(Session *pSession, const QString &nick);

    void addPrefix(const QChar &prefix);
    void removePrefix(const QChar &prefix);

    // sets and gets only the nickname, without any prefixes
    void setNickname(const QString &nick) { m_nickname = nick; }
    QString getNickname() { return m_nickname; }

    QString getProperNickname();
    QString getFullNickname();
    QChar getPrefix();
    void setPriority(int p) { m_priority = p; }
    int getPriority() { return m_priority; }
};

} // end namespace
