/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include "cv/ChannelUser.h"
#include "cv/Parser.h"

namespace cv {

// parses the input nick into nickname, and
// prefixes and user/host (if applicable)
ChannelUser::ChannelUser(QExplicitlySharedDataPointer<Session> pSharedSession, const QString &nick)
    : m_pSharedSession(pSharedSession)
{
    m_nickname = nick.section('!', 0, 0);

    // get the prefixes from the nickname (if any)
    int numPrefixes = 0;
    for(int i = 0; i < m_nickname.size() && m_pSharedSession->isNickPrefix(m_nickname[i]); ++i)
    {
        m_prefixes += m_nickname[i];
        ++numPrefixes;
    }

    // remove the prefixes from m_nickname
    if(numPrefixes > 0)
    {
        m_nickname.remove(0, numPrefixes);
    }

    // get the user and host, if applicable
    if(nick.indexOf('!') >= 0)
    {
        QString userAndHost = nick.section('!', 1);
        m_user = userAndHost.section('@', 0, 0);
        m_host = userAndHost.section('@', 1);
    }
}

// adds the given prefix to the user, unless it's
// already there or it isn't a valid prefix
void ChannelUser::addPrefix(const QChar &prefixToAdd)
{
    if(!m_pSharedSession->isNickPrefix(prefixToAdd))
        return;

    // insert the prefix in the right spot
    for(int i = 0; i < m_prefixes.size(); ++i)
    {
        int compareVal = m_pSharedSession->compareNickPrefixes(m_prefixes[i], prefixToAdd);
        if(compareVal > 0)
        {
            m_prefixes.insert(i, prefixToAdd);
            return;
        }
        else if(compareVal == 0)
        {
            return;;
        }
    }

    m_prefixes.append(prefixToAdd);
}

// removes the given prefix from the user, if it exists
void ChannelUser::removePrefix(const QChar &prefix)
{
    m_prefixes.remove(prefix);
}

// returns only the nickname, without any prefixes
QString ChannelUser::getNickname()
{
    return m_nickname;
}

// returns the nickname with the most powerful prefix (if any)
// prepended to it
QString ChannelUser::getProperNickname()
{
    QString name;

    if(!m_prefixes.isEmpty())
    {
        name += m_prefixes[0];
    }
    name += m_nickname;

    return name;
}

// returns the nickname with all prefixes (if any) prepended
// to it
QString ChannelUser::getFullNickname()
{
    QString name;

    if(!m_prefixes.isEmpty())
    {
        name += m_prefixes;
    }
    name += m_nickname;

    return name;
}

// returns the most powerful prefix of the nickname,
// or '\0' if there is none
QChar ChannelUser::getPrefix()
{
    if(!m_prefixes.isEmpty())
    {
        return m_prefixes[0];
    }

    return '\0';
}

} // end namespace