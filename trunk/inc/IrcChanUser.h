/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QString>
#include "IrcServerInfoService.h"

class IrcChanUser
{
	QExplicitlySharedDataPointer<IrcServerInfoService>
					m_pSharedService;
	
	// msg prefix: nick!user@host
	QString			m_nickname;
	QString			m_prefixes;
	QString			m_user;
	QString			m_host;
	
	// priority of the nickname for tab-completion
	int				m_priority;

public:
	// parses the input nick into nickname, and
	// prefixes and user/host (if applicable)
	IrcChanUser(QExplicitlySharedDataPointer<IrcServerInfoService> pSharedService, const QString &nick);
	
	// adds the given prefix to the user, unless it's
	// already there or it isn't a valid prefix
	void AddPrefix(const QChar &prefix);
	
	// removes the given prefix from the user, if it exists
	void RemovePrefix(const QChar &prefix);
	
	// sets the user's nickname
	void SetNickname(const QString &nick) { m_nickname = nick; }
	
	// returns only the nickname, without any prefixes
	QString GetNickname();
	
	// returns the nickname with the most powerful prefix (if any)
	// prepended to it
	QString GetProperNickname();
	
	// returns the nickname with all prefixes (if any) prepended
	// to it
	QString GetFullNickname();
	
	// returns the most powerful prefix of the nickname,
	// or '\0' if there is none
	QChar GetPrefix();
	
	void SetPriority(int p) { m_priority = p; }
	
	int GetPriority() { return m_priority; }
};
