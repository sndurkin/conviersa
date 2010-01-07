/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QString>
#include "Session.h"

namespace irc {

class ChannelUser
{
    QExplicitlySharedDataPointer<Session>
                m_pSharedSession;

    // msg prefix: nick!user@host
    QString     m_nickname;
    QString     m_prefixes;
    QString     m_user;
    QString     m_host;

    // priority of the nickname for tab-completion
    int         m_priority;

public:
    // parses the input nick into nickname, and
    // prefixes and user/host (if applicable)
    ChannelUser(QExplicitlySharedDataPointer<Session> pSharedSession, const QString &nick);

    // adds the given prefix to the user, unless it's
    // already there or it isn't a valid prefix
    void addPrefix(const QChar &prefix);

    // removes the given prefix from the user, if it exists
    void removePrefix(const QChar &prefix);

    // sets the user's nickname
    void setNickname(const QString &nick) { m_nickname = nick; }

    // returns only the nickname, without any prefixes
    QString getNickname();

    // returns the nickname with the most powerful prefix (if any)
    // prepended to it
    QString getProperNickname();

    // returns the nickname with all prefixes (if any) prepended
    // to it
    QString getFullNickname();

    // returns the most powerful prefix of the nickname,
    // or '\0' if there is none
    QChar getPrefix();

    void setPriority(int p) { m_priority = p; }

    int getPriority() { return m_priority; }
};

} // end namespace
