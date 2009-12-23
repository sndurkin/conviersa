/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include "IIrcWindow.h"
#include "IrcParser.h"

class IrcChanListWindow;
class IrcChanWindow;
class IrcPrivWindow;

class IrcStatusWindow : public IIrcWindow
{
	Q_OBJECT
	
protected:
	bool					m_populatingUserList;
	bool					m_sentListStopMsg;
	
	IrcChanListWindow *		m_pChanListWin;
	QList<IrcChanWindow *>	m_chanList;
	QList<IrcPrivWindow *>	m_privList;
	
public:
	IrcStatusWindow(const QString &title = tr("Server Window"),
			const QSize &size = QSize(500, 300));
	virtual ~IrcStatusWindow();
	
	int GetIrcWindowType();
	
	// handles the data received from the Connection class
	void HandleData(QString &data);
	
	// returns a pointer to the IIrcWindow if it exists,
	// 	and is a child of this status window (meaning
	//	it can only be a channel or PM window)
	// returns NULL otherwise
	IIrcWindow *GetChildIrcWindow(const QString &name);
	
	// returns a list of all IIrcWindows under the IrcStatusWindow
	QList<IIrcWindow *> GetChildIrcWindows();
	
	// returns a list of all IrcChanWindows that are currently
	// being managed in the server
	QList<IrcChanWindow *> GetChannels();
	
	// returns a list of all IrcPrivWindows that are currently
	// being managed in the server
	QList<IrcPrivWindow *> GetPrivateMessages();
	
	// adds a channel to the list
	void AddChanWindow(IrcChanWindow *pChan);
	
	// adds a PM window to the list
	void AddPrivWindow(IrcPrivWindow *pPriv);

protected:
	void HandleTab();
	
	// handles child widget events
	bool eventFilter(QObject *obj, QEvent *event);

private:
	// numeric messages
	void Handle001Numeric(const IrcParser::IrcMessage &msg);
	void Handle002Numeric(const IrcParser::IrcMessage &msg);
	void Handle005Numeric(const IrcParser::IrcMessage &msg);
	void Handle301Numeric(const IrcParser::IrcMessage &msg);
	void Handle317Numeric(const IrcParser::IrcMessage &msg);
	void Handle321Numeric(const IrcParser::IrcMessage &msg);
	void Handle322Numeric(const IrcParser::IrcMessage &msg);
	void Handle323Numeric(const IrcParser::IrcMessage &msg);
	void Handle330Numeric(const IrcParser::IrcMessage &msg);
	void Handle332Numeric(const IrcParser::IrcMessage &msg);
	void Handle333Numeric(const IrcParser::IrcMessage &msg);
	void Handle353Numeric(const IrcParser::IrcMessage &msg);
	void Handle366Numeric(const IrcParser::IrcMessage &msg);
	void Handle401Numeric(const IrcParser::IrcMessage &msg);
	
	// other messages
	void HandleInviteMsg(const IrcParser::IrcMessage &msg);
	void HandleJoinMsg(const IrcParser::IrcMessage &msg);
	void HandleKickMsg(const IrcParser::IrcMessage &msg);
	void HandleModeMsg(const IrcParser::IrcMessage &msg);
	void HandleNickMsg(const IrcParser::IrcMessage &msg);
	void HandleNoticeMsg(const IrcParser::IrcMessage &msg);
	void HandlePartMsg(const IrcParser::IrcMessage &msg);
	void HandlePongMsg(const IrcParser::IrcMessage &msg);
	void HandlePrivMsg(const IrcParser::IrcMessage &msg);
	void HandleQuitMsg(const IrcParser::IrcMessage &msg);
	void HandleTopicMsg(const IrcParser::IrcMessage &msg);
	void HandleWallopsMsg(const IrcParser::IrcMessage &msg);

public slots:
	// handles a disconnection fired from the Connection object
	void HandleDisconnect();
	
	// removes a channel window from the list
	void RemoveChanWindow(IrcChanWindow *pChanWin);
	
	// removes a PM window from the list
	void RemovePrivWindow(IrcPrivWindow *pChanWin);
};
