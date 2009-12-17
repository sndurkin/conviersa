#ifndef IRCCHANWINDOW_H
#define IRCCHANWINDOW_H

#include <QString>
#include "IIrcWindow.h"

class QListWidget;
class QSplitter;
class QMutex;
class IrcChanUser;

// subject to change
const unsigned int MAX_NICK_PRIORITY = 1;

class IrcChanWindow : public IIrcWindow
{
	Q_OBJECT
	
protected:
	QListWidget *		m_pUserList;
	QSplitter *			m_pSplitter;
	
	QList<IrcChanUser *>	m_users;
	
	bool				m_inChannel;

public:
	IrcChanWindow(QExplicitlySharedDataPointer<IrcServerInfoService> pSharedService,
				QExplicitlySharedDataPointer<Connection> pSharedConn,
				const QString &title = tr("Untitled"),
				const QSize &size = QSize(500, 300));
	
	bool IsInChannel() { return m_inChannel; }
	
	int GetIrcWindowType();
	
	// lets the user know that he is back inside the channel,
	// whose window was already open when it was rejoined
	void JoinChannel();
	
	// lets the user know that he is no longer in the channel,
	// but still has the window open
	void LeaveChannel();
	
	// returns true if the user is in the channel,
	// returns false otherwise
	bool HasUser(const QString &user);
	
	// adds the user to the channel's userlist in the proper place
	//
	// user holds the nickname, and can include any number
	// of prefixes, as well as the user and host
	//
	// returns true if the user was added,
	// returns false otherwise
	bool AddUser(const QString &user);
	
	// removes the user from the channel's userlist
	//
	// returns true if the user was removed,
	// returns false otherwise
	bool RemoveUser(const QString &user);
	
	// changes the user's nickname from oldNick to newNick, unless
	// the user is not in the channel
	void ChangeUserNick(const QString &oldNick, const QString &newNick);
	
	// adds the specified prefix to the user, and updates the user list display
	// (if necessary)
	void AddPrefixToUser(const QString &user, const QChar &prefixToAdd);
	
	// removes the specified prefix from the user, and updates the user list
	// display (if necessary) - assuming the user has the given prefix
	void RemovePrefixFromUser(const QString &user, const QChar &prefixToRemove);
	
	// returns the number of users currently in the channel
	int GetUserCount() { return m_users.size(); }
	
protected:
	void HandleTab();
	void closeEvent(QCloseEvent *event);
	
private:
	// adds a user by pointer to IrcChanUser
	bool AddUser(IrcChanUser *pUser);
	
	// removes a user by pointer to IrcChanUser
	void RemoveUser(IrcChanUser *pUser);
	
	// finds the user within the channel, based on nickname
	// (regardless if there are prefixes or a user/host in it)
	//
	// returns a pointer to the IrcChanUser if found in the channel,
	// returns NULL otherwise
	IrcChanUser *FindUser(const QString &user);

signals:
	// signifies that the window is closing - this is *only*
	// for IrcStatusWindow to use
	void ChanWindowClosing(IrcChanWindow *pWin);
	
public slots:
	// handles a disconnection fired from the Connection object
	void HandleDisconnect();
};

#endif
