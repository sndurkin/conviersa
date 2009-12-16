#include <QWebView>
#include <QAction>
#include <QListWidget>
#include <QSplitter>
#include <QMutex>
#include "IrcTypes.h"
#include "IrcChanWindow.h"
#include "IrcChanUser.h"
#include "IrcStatusWindow.h"
#include "IrcServerInfoService.h"
#include "WindowManager.h"
#include "Connection.h"

#include <QScrollBar>
#include <QTextEdit>

IrcChanWindow::IrcChanWindow(QExplicitlySharedDataPointer<IrcServerInfoService> pSharedService,
					QExplicitlySharedDataPointer<Connection> pSharedConn,
					const QString &title/* = tr("Untitled")*/,
					const QSize &size/* = QSize(500, 300)*/)
	: IIrcWindow(title, size),
	  m_inChannel(false)
{
	m_pSharedConn = pSharedConn;
	m_pSharedService = pSharedService;
	
	QFont defFont("Fixedsys", 9);
	
	m_pSplitter = new QSplitter(this);
	m_pUserList = new QListWidget;
	m_pUserList->setFont(defFont);
	
	m_pSplitter->addWidget(m_pOutput);
	m_pSplitter->addWidget(m_pUserList);
	
	if(size.width() > 150)
	{
		QList<int> sizes;
		sizes.append(size.width() - 150);
		sizes.append(150);
		m_pSplitter->setSizes(sizes);
	}
	m_pSplitter->setStretchFactor(0, 1);
	
	m_pVLayout->addWidget(m_pSplitter, 1);
	m_pVLayout->addWidget(m_pInput);
	m_pVLayout->setSpacing(5);
	m_pVLayout->setContentsMargins(2, 2, 2, 2);	
	
	setLayout(m_pVLayout);
	
	QObject::connect(m_pSharedConn.data(), SIGNAL(Disconnected()), this, SLOT(HandleDisconnect()));
}

int IrcChanWindow::GetIrcWindowType()
{
	return IRC_CHAN_WIN_TYPE;
}

// lets the user know that he is back inside the channel,
// whose window was already open when it was rejoined
void IrcChanWindow::JoinChannel()
{
	m_inChannel = true;
}

// lets the user know that he is no longer in the channel,
// but still has the window open
void IrcChanWindow::LeaveChannel()
{
	m_inChannel = false;
	while(m_users.size() > 0)
	{
		delete m_users.takeAt(0);
		m_pUserList->takeItem(0);
	}
}

// returns true if the user is in the channel,
// returns false otherwise
bool IrcChanWindow::HasUser(const QString &user)
{
	return (FindUser(user) != NULL);
}

// adds the user to the channel's userlist in the proper place
//
// user holds the nickname, and can include any number
// of prefixes, as well as the user and host
//
// returns true if the user was added,
// returns false otherwise
bool IrcChanWindow::AddUser(const QString &user)
{
	IrcChanUser *pNewUser = new (std::nothrow) IrcChanUser(m_pSharedService, user);
	if(!pNewUser)
	{
		PrintError("Allocation of a new user failed.");
		return false;
	}
	
	if(!AddUser(pNewUser))
	{
		delete pNewUser;
		return false;
	}
	
	return true;
}

// removes the user from the channel's userlist
bool IrcChanWindow::RemoveUser(const QString &user)
{
	for(int i = 0; i < m_users.size(); ++i)
	{
		if(user.compare(m_users[i]->GetNickname(), Qt::CaseInsensitive) == 0)
		{
			delete m_users.takeAt(i);
			m_pUserList->takeItem(i);
			return true;
		}
	}
	
	return false;
}

// changes the user's nickname from oldNick to newNick, unless
// the user is not in the channel
void IrcChanWindow::ChangeUserNick(const QString &oldNick, const QString &newNick)
{
	IrcChanUser *pNewUser = FindUser(oldNick);
	if(!pNewUser)
		return;
	
	IrcChanUser *pUser = new (std::nothrow) IrcChanUser(m_pSharedService, newNick);
	if(!pUser)
		return;
	
	for(int i = 0; i < m_users.size(); ++i)
	{
		if(m_users[i] == pNewUser)
		{
			RemoveUser(pNewUser);
			pNewUser->SetNickname(pUser->GetNickname());
			AddUser(pNewUser);
			break;
		}
	}
	
	delete pUser;
}

// adds the specified prefix to the user, and updates the user list display
// (if necessary)
void IrcChanWindow::AddPrefixToUser(const QString &user, const QChar &prefixToAdd)
{
	IrcChanUser *pUser = FindUser(user);
	if(pUser)
	{
		// could be faster, but whatever; i'm lazy with my own API
		RemoveUser(pUser);
		pUser->AddPrefix(prefixToAdd);
		AddUser(pUser);
	}
}

// removes the specified prefix from the user, and updates the user list
// display (if necessary) - assuming the user has the given prefix
void IrcChanWindow::RemovePrefixFromUser(const QString &user, const QChar &prefixToRemove)
{
	IrcChanUser *pUser = FindUser(user);
	if(pUser)
	{
		// again, i'm lazy
		RemoveUser(pUser);
		pUser->RemovePrefix(prefixToRemove);
		AddUser(pUser);
	}
}

void IrcChanWindow::HandleTab()
{
	QString text = GetInputText();
	int idx = m_pInput->textCursor().position();
	
	// find the beginning of the word that the user is
	// trying to tab-complete
	while(idx >= 0 && text[idx] != ' ') --idx;
	
	// now capture the word
	QString word;
	int idxSpace = text.indexOf(' ', idx);
	if(idxSpace < 0)
	{
		word = text.mid(idx);
	}
	else
	{
		word = text.mid(idx, idxSpace - idx);
	}
	
	// if there is no word to complete, then exit
	if(word.isEmpty())
	{
		// todo: beep?
		return;
	}
	
	// now search in the user list for the name
	for(int i = MAX_NICK_PRIORITY; i >= 0; --i)
	{ 
		for(int j = 0; j < m_users.size(); ++j)
		{
			if(m_users[j]->GetPriority() == i)
			{
				
			}
		}
	}
}

void IrcChanWindow::closeEvent(QCloseEvent *event)
{
	emit ChanWindowClosing(this);
	
	if(m_inChannel)
	{
		LeaveChannel();
		QString textToSend = "PART ";
		textToSend += GetWindowName();
		m_pSharedConn->Send(textToSend);
	}
	
	return IWindow::closeEvent(event);
}

// adds a user by pointer to IrcChanUser
bool IrcChanWindow::AddUser(IrcChanUser *pNewUser)
{
	QListWidgetItem *pListItem = new (std::nothrow) QListWidgetItem(pNewUser->GetProperNickname());
	if(!pListItem)
		return false;
	
	for(int i = 0; i < m_users.size(); ++i)
	{
		int compareVal = m_pSharedService->CompareNickPrefixes(m_users[i]->GetPrefix(), pNewUser->GetPrefix());
		if(compareVal > 0)
		{
			m_pUserList->insertItem(i, pListItem);
			m_users.insert(i, pNewUser);
			return true;
		}
		else if(compareVal == 0)
		{
			compareVal = QString::compare(m_users[i]->GetNickname(), pNewUser->GetNickname(), Qt::CaseInsensitive);
			if(compareVal > 0)
			{
				m_pUserList->insertItem(i, pListItem);
				m_users.insert(i, pNewUser);
				return true;
			}
			else if(compareVal == 0)
			{
				// already in the list
				return false;
			}
		}
	}
	
	m_pUserList->addItem(pListItem);
	m_users.append(pNewUser);
	return true;
}

// removes a user by pointer to IrcChanUser
void IrcChanWindow::RemoveUser(IrcChanUser *pUser)
{
	for(int i = 0; i < m_users.size(); ++i)
	{
		if(m_users[i] == pUser)
		{
			delete m_pUserList->takeItem(i);
			m_users.removeAt(i);
			break;
		}
	}
}

// finds the user within the channel, based on nickname
// (regardless if there are prefixes or a user/host in it)
//
// returns a pointer to the IrcChanUser if found in the channel,
// returns NULL otherwise
IrcChanUser *IrcChanWindow::FindUser(const QString &user)
{
	IrcChanUser ircChanUser(m_pSharedService, user);
	for(int i = 0; i < m_users.size(); ++i)
	{
		if(QString::compare(m_users[i]->GetNickname(), ircChanUser.GetNickname(), Qt::CaseInsensitive) == 0)
		{
			return m_users[i];
		}
	}
	
	return NULL;
}

void IrcChanWindow::HandleDisconnect()
{
	PrintOutput("* Disconnected");
	m_pUserList->setEnabled(false);
}
