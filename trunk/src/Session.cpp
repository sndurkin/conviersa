/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include "Session.h"

namespace cv { namespace irc {

Session::Session()
    : QSharedData(),
      m_attached(false),
      m_nick("seand`")
{ }

void Session::attachToServer(const QString &host, int port)
{
    m_host = host;
    m_port = port;
    m_prefixRules = "o@v+";
    m_attached = true;
}

void Session::detachFromServer()
{
    m_attached = false;
}

// sets the prefix rules
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

} } // end namespaces

