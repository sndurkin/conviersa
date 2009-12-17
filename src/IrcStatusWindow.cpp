#include <QWebView>
#include <QMutex>
#include <QDateTime>
#include "qext.h"
#include "IrcTypes.h"
#include "IrcStatusWindow.h"
#include "IrcChanListWindow.h"
#include "IrcChanWindow.h"
#include "IrcPrivWindow.h"
#include "IrcServerInfoService.h"
#include "IrcWindowScrollBar.h"
#include "WindowManager.h"
#include "ConfigManager.h"

using namespace IrcParser;

#define DEBUG_MESSAGES 0

IrcStatusWindow::IrcStatusWindow(const QString &title/* = tr("Server Window")*/,
				const QSize &size/* = QSize(500, 300)*/)
	: IIrcWindow(title, size),
	  m_populatingUserList(false),
	  m_pChanListWin(NULL)
{
	m_pVLayout->addWidget(m_pOutput);
	m_pVLayout->addWidget(m_pInput);
	m_pVLayout->setSpacing(5);
	m_pVLayout->setContentsMargins(2, 2, 2, 2);
	setLayout(m_pVLayout);
	
	m_pSharedService = new IrcServerInfoService();
	m_pSharedConn = new Connection(this, m_pCodec);
	
	QObject::connect(m_pSharedConn.data(), SIGNAL(Disconnected()), this, SLOT(HandleDisconnect()));
}

IrcStatusWindow::~IrcStatusWindow()
{
	m_pSharedConn->Disconnect();
}

int IrcStatusWindow::GetIrcWindowType()
{
	return IRC_STATUS_WIN_TYPE;
}

// handles the data received from the Connection class
void IrcStatusWindow::HandleData(QString &data)
{
#if DEBUG_MESSAGES
	QString blah = data;
	blah.remove(blah.size()-2,2);
	PrintDebug(blah);
#endif
	
	IrcMessage msg = ParseData(data);
	
	if(msg.m_isNumeric)
	{
		switch(msg.m_command)
		{
			case 1:
			{
				Handle001Numeric(msg);				
				PrintOutput(GetNumericText(msg));
				break;
			}
			case 2:
			{
				Handle002Numeric(msg);				
				PrintOutput(GetNumericText(msg));
				break;
			}
			case 5:
			{
				Handle005Numeric(msg);
				PrintOutput(GetNumericText(msg));
				break;
			}
			// RPL_AWAY
			case 301:
			{
				Handle301Numeric(msg);
				break;
			}
			/*// RPL_USERHOST
			case 302:
			{
				
				break;
			}
			// RPL_ISON
			case 303:
			{
				
				break;
			}*/
			// RPL_WHOISIDLE
			case 317:
			{
				Handle317Numeric(msg);
				break;
			}
			// RPL_LISTSTART
			case 321:
			{
				Handle321Numeric(msg);
				break;
			}
			// RPL_LIST
			case 322:
			{
				Handle322Numeric(msg);
				break;
			}
			// RPL_LISTEND
			case 323:
			{
				Handle323Numeric(msg);
				break;
			}
			// RPL_WHOISACCOUNT
			case 330:
			{
				Handle330Numeric(msg);
				break;
			}
			// RPL_TOPIC
			case 332:
			{
				Handle332Numeric(msg);
				break;
			}
			// states when topic was last set
			case 333:
			{
				Handle333Numeric(msg);
				break;
			}
			// RPL_NAMREPLY
			case 353:
			{
				Handle353Numeric(msg);
				break;
			}
			// RPL_ENDOFNAMES
			case 366:
			{
				Handle366Numeric(msg);
				break;
			}
			// ERR_NOSUCKNICK
			case 401:
			{
				Handle401Numeric(msg);				
				break;
			}
			// ERR_CANNOTSENDTOCHAN
			case 404:
			{
				// handles 404 numeric messages too
				Handle401Numeric(msg);				
				break;
			}
			default:
			{
				// the following are not meant to be handled,
				// but only printed:
				// 	003
				// 	004
				// 	305
				// 	306
				//PrintOutput(ConvertDataToHtml(GetNumericText(msg)));
				PrintOutput(GetNumericText(msg));
			}
		}
	}
	else
	{
		switch(msg.m_command)
		{
			case IRC_COMMAND_ERROR:
			{
				// prints the error message
				PrintOutput(msg.m_params[0]);
				break;
			}
			case IRC_COMMAND_INVITE:
			{
				HandleInviteMsg(msg);
				break;
			}
			case IRC_COMMAND_JOIN:
			{
				HandleJoinMsg(msg);
				break;
			}
			case IRC_COMMAND_KICK:
			{
				HandleKickMsg(msg);
				break;
			}
			case IRC_COMMAND_MODE:
			{
				HandleModeMsg(msg);
				break;
			}
			case IRC_COMMAND_NICK:
			{
				HandleNickMsg(msg);
				break;
			}
			case IRC_COMMAND_NOTICE:
			{
				HandleNoticeMsg(msg);
				break;
			}
			case IRC_COMMAND_PART:
			{
				HandlePartMsg(msg);
				break;
			}
			case IRC_COMMAND_PING:
			{
				m_pSharedConn->Send("PONG :" + msg.m_params[0]);
				break;
			}
			case IRC_COMMAND_PONG:
			{
				HandlePongMsg(msg);
				break;
			}
			case IRC_COMMAND_PRIVMSG:
			{
				HandlePrivMsg(msg);
				break;
			}
			case IRC_COMMAND_QUIT:
			{
				HandleQuitMsg(msg);
				break;
			}
			case IRC_COMMAND_TOPIC:
			{
				HandleTopicMsg(msg);
				break;
			}
			case IRC_COMMAND_WALLOPS:
			{
				HandleWallopsMsg(msg);
				break;
			}
			default:
			{
				// print the whole raw line
				PrintOutput(data);
			}
		}
	}
}

// returns a pointer to the IIrcWindow if it exists
// 	(and is a child of this status window)
// returns NULL otherwise
IIrcWindow *IrcStatusWindow::GetChildIrcWindow(const QString &name)
{
	for(int i = 0; i < m_chanList.size(); ++i)
	{
		if(name.compare(m_chanList[i]->GetWindowName(), Qt::CaseInsensitive) == 0)
		{
			return m_chanList[i];
		}
	}
	
	for(int i = 0; i < m_privList.size(); ++i)
	{
		if(name.compare(m_privList[i]->GetWindowName(), Qt::CaseInsensitive) == 0)
		{
			return m_privList[i];
		}
	}
	
	return NULL;
}

// returns a list of all IrcChanWindows that are currently
// being managed in the server
QList<IrcChanWindow *> IrcStatusWindow::GetChannels()
{
	return m_chanList;
}

// returns a list of all IrcPrivWindows that are currently
// being managed in the server
QList<IrcPrivWindow *> IrcStatusWindow::GetPrivateMessages()
{
	return m_privList;
}

// adds a channel to the list
void IrcStatusWindow::AddChanWindow(IrcChanWindow *pChanWin)
{
	if(m_pManager)
		m_pManager->AddWindow(pChanWin, m_pManager->GetItemFromWindow(this));
	m_chanList.append(pChanWin);
	QObject::connect(pChanWin, SIGNAL(ChanWindowClosing(IrcChanWindow *)), 
				this, SLOT(RemoveChanWindow(IrcChanWindow *)));
}

// removes a channel from the list
void IrcStatusWindow::RemoveChanWindow(IrcChanWindow *pChanWin)
{
	m_chanList.removeOne(pChanWin);
}

// adds a PM window to the list
void IrcStatusWindow::AddPrivWindow(IrcPrivWindow *pPrivWin)
{
	if(m_pManager)
		m_pManager->AddWindow(pPrivWin, m_pManager->GetItemFromWindow(this));
	m_privList.append(pPrivWin);
	QObject::connect(pPrivWin, SIGNAL(PrivWindowClosing(IrcPrivWindow *)), 
				this, SLOT(RemovePrivWindow(IrcPrivWindow *)));
}

// removes a PM window from the list
void IrcStatusWindow::RemovePrivWindow(IrcPrivWindow *pPrivWin)
{
	m_privList.removeOne(pPrivWin);
}

void IrcStatusWindow::HandleTab()
{
	QString text = GetInputText();
	int idx = m_pInput->textCursor().position();
}

// handles child widget events
bool IrcStatusWindow::eventFilter(QObject *obj, QEvent *event)
{
	// just for monitoring when it's closed
	if(obj == m_pChanListWin && event->type() == QEvent::Close)
	{
		m_pChanListWin = NULL;
	}
	
	return IIrcWindow::eventFilter(obj, event);
}

void IrcStatusWindow::Handle001Numeric(const IrcMessage &msg)
{
	// msg.m_params[0]: my nick
	// msg.m_params[1]: "Welcome to the <server name> IRC Network, <nick>[!user@host]"
	//
	// check to make sure nickname hasn't changed; some or all servers apparently don't
	// send you a NICK message when your nickname conflicts with another user upon
	// first entering the server, and you try to change it
	if(m_pSharedService->GetNick().compare(msg.m_params[0], Qt::CaseInsensitive) != 0)
	{
		m_pSharedService->SetNick(msg.m_params[0]);
	}

	QString header = "Welcome to the ";
	if(msg.m_params[1].startsWith(header, Qt::CaseInsensitive))
	{
		int idx = msg.m_params[1].indexOf(' ', header.size(), Qt::CaseInsensitive);
		if(idx >= 0)
		{
			// change name of the window to the name of the network
			QString networkName = msg.m_params[1].mid(header.size(), idx - header.size());
			SetTitle(networkName);
			SetWindowName(networkName);
		}
	}
}

void IrcStatusWindow::Handle002Numeric(const IrcMessage &msg)
{
	// msg.m_params[0]: my nick
	// msg.m_params[1]: "Your host is ..."
	QString header = "Your host is ";
	QString hostStr = msg.m_params[1].section(',', 0, 0);
	if(hostStr.startsWith(header))
	{
		m_pSharedService->SetHost(hostStr.mid(header.size()));
	}
}

void IrcStatusWindow::Handle005Numeric(const IrcMessage &msg)
{
	// we only go to the second-to-last parameter,
	// because the last parameter holds "are supported
	// by this server"
	for(int i = 1; i < msg.m_paramsNum-1; ++i)
	{
		if(msg.m_params[i].startsWith("PREFIX=", Qt::CaseInsensitive))
		{
			m_pSharedService->SetPrefixRules(GetPrefixRules(msg.m_params[i]));
		}
		else if(msg.m_params[i].compare("NAMESX", Qt::CaseInsensitive) == 0)
		{
			// lets the server know we support multiple nick prefixes
			//
			// todo: UHNAMES?
			m_pSharedConn->Send("PROTOCTL NAMESX");
		}
		else if(msg.m_params[i].startsWith("CHANMODES=", Qt::CaseInsensitive))
		{
			m_pSharedService->SetChanModes(msg.m_params[i].section('=', 1));
		}
	}
}

void IrcStatusWindow::Handle301Numeric(const IrcMessage &msg)
{
	// msg.m_params[0]: my nick
	// msg.m_params[1]: nick
	// msg.m_params[2]: away message
        QString textToPrint = QString("%1 is away: %2")
                                .arg(msg.m_params[1])
                                .arg(ConvertDataToHtml(msg.m_params[2]));
	PrintOutput(textToPrint);
}

void IrcStatusWindow::Handle317Numeric(const IrcMessage &msg)
{
	// msg.m_params[0]: my nick
	// msg.m_params[1]: nick
	// msg.m_params[2]: seconds
	// two options here:
        //	1)      msg.m_params[3]: "seconds idle"
        //	2)      msg.m_params[3]: unix time
	//		msg.m_params[4]: "seconds idle, signon time"
	
	// get the number of idle seconds first, convert
	// to h, m, s format	
	bool conversionOk;
	uint numSecs = msg.m_params[2].toInt(&conversionOk);
	if(conversionOk)
	{
		QString textToPrint = QString("%1 has been idle ").arg(msg.m_params[1]);
		
		// 24 * 60 * 60 = 86400
		uint numDays = numSecs / 86400;
		if(numDays)
		{
			textToPrint += QString::number(numDays);
			if(numDays == 1)
			{
				textToPrint += "day ";
			}
			else
			{
				textToPrint += "days ";
			}
			numSecs = numSecs % 86400;
		}
		
		// 60 * 60 = 3600
		uint numHours = numSecs / 3600;
		if(numHours)
		{
			textToPrint += QString::number(numHours);
			if(numHours == 1)
			{
				textToPrint += "hr ";
			}
			else
			{
				textToPrint += "hrs ";
			}
			numSecs = numSecs % 3600;
		}
		
		uint numMinutes = numSecs / 60;
		if(numMinutes)
		{
			textToPrint += QString::number(numMinutes);
			if(numMinutes == 1)
			{
				textToPrint += "min ";
			}
			else
			{
				textToPrint += "mins ";
			}
			numSecs = numSecs % 60;
		}
		
		if(numSecs)
		{
			textToPrint += QString::number(numSecs);
			if(numSecs == 1)
			{
				textToPrint += "sec ";
			}
			else
			{
				textToPrint += "secs ";
			}
		}
		
		// remove trailing space
		textToPrint.remove(textToPrint.size()-1, 1);
		
		// right now this will only support 5 parameters
		// (1 extra for the signon time), but i can easily
		// add support for more later
		if(msg.m_paramsNum > 4)
		{
			uint uTime = msg.m_params[3].toInt(&conversionOk);
			if(conversionOk)
			{
				QDateTime dt;
				dt.setTime_t(uTime);
				
				QString strDate = dt.toString("ddd MMM dd");
				QString strTime = dt.toString("hh:mm:ss");
				
				textToPrint += QString(", signed on %1 %2").arg(strDate).arg(strTime);
			}
		}
		
		PrintOutput(textToPrint);
	}
}

void IrcStatusWindow::Handle321Numeric(const IrcMessage &msg)
{
	if(!m_pChanListWin)
	{	
		m_pChanListWin = new (std::nothrow) IrcChanListWindow(m_pSharedConn);
		if(!m_pChanListWin)
			return;
		
		m_pManager->AddWindow(m_pChanListWin, m_pManager->GetItemFromWindow(this));
		QString title = QString("Channel List - %1").arg(GetWindowName());
		m_pChanListWin->SetTitle(title);
		
		// todo: fix and put inside my own event system
		m_pChanListWin->installEventFilter(this);
	}
	else
	{
		m_pChanListWin->ClearList();
		QString title = QString("Channel List - %1").arg(GetWindowName());
		m_pChanListWin->SetTitle(title);
	}
	
	m_pChanListWin->BeginPopulatingList();
}

void IrcStatusWindow::Handle322Numeric(const IrcMessage &msg)
{
	// msg.m_params[0]: my nick
	// msg.m_params[1]: channel
	// msg.m_params[2]: number of users
	// msg.m_params[3]: topic
	if(m_pChanListWin)
	{
		m_pChanListWin->AddChannel(msg.m_params[1], msg.m_params[2], msg.m_params[3]);
	}
	else
	{
		if(!m_sentListStopMsg)
		{
			m_sentListStopMsg = true;
			m_pSharedConn->Send("LIST STOP");
		}
	}
}

void IrcStatusWindow::Handle323Numeric(const IrcMessage &msg)
{
	if(m_pChanListWin)
	{
		m_pChanListWin->EndPopulatingList();
	}
	
	m_sentListStopMsg = false;
}

void IrcStatusWindow::Handle330Numeric(const IrcMessage &msg)
{
	// msg.m_params[0]: my nick
	// msg.m_params[1]: nick
	// msg.m_params[2]: login/auth
	// msg.m_params[3]: "is logged in as"
	QString textToPrint = QString("%1 %2: %3").arg(msg.m_params[1])
								.arg(msg.m_params[3])
								.arg(msg.m_params[2]);
	PrintOutput(textToPrint);
}

void IrcStatusWindow::Handle332Numeric(const IrcMessage &msg)
{
	// msg.m_params[0]: my nick
	// msg.m_params[1]: channel
	// msg.m_params[2]: topic
	IIrcWindow *pChanWin = GetChildIrcWindow(msg.m_params[1]);
	if(pChanWin)
	{
		QString titleWithTopic = QString("%1: %2").arg(pChanWin->GetWindowName())
									.arg(StripCodes(msg.m_params[2]));
		pChanWin->SetTitle(titleWithTopic);
		
		QColor topicColor(g_pCfgManager->GetOptionValue("colors.ini", "topic"));
		QString textToPrint = QString("* Topic is: %1").arg(msg.m_params[2]);
		pChanWin->PrintOutput(textToPrint, topicColor);
	}
	else
	{
		PrintOutput(GetNumericText(msg));
	}
}

void IrcStatusWindow::Handle333Numeric(const IrcMessage &msg)
{
	// msg.m_params[0]: my nick
	// msg.m_params[1]: channel
	// msg.m_params[2]: nick
	// msg.m_params[3]: unix time
	IIrcWindow *pChanWin = GetChildIrcWindow(msg.m_params[1]);
	
	bool conversionOk;
	uint uTime = msg.m_params[3].toInt(&conversionOk);
	if(conversionOk)
	{
		QDateTime dt;
		dt.setTime_t(uTime);
		QString strDate = dt.toString("ddd MMM dd");
		QString strTime = dt.toString("hh:mm:ss");
		
		QString textToPrint;
		if(pChanWin)
		{
			textToPrint += "* Topic set by ";
		}
		else
		{
			textToPrint += QString("%1 topic set by ").arg(msg.m_params[1]);
		}
		
		textToPrint += QString("%1 on %2 %3").arg(msg.m_params[2]).arg(strDate).arg(strTime);
		
		if(pChanWin)
		{
			pChanWin->PrintOutput(textToPrint, QColor(g_pCfgManager->GetOptionValue("colors.ini", "topic")));
		}
		else
		{
			PrintOutput(textToPrint);
		}
	}
}

void IrcStatusWindow::Handle353Numeric(const IrcMessage &msg)
{
	// msg.m_params[0]: my nick
	// msg.m_params[1]: "=" | "*" | "@"
	// msg.m_params[2]: channel
	// msg.m_params[3]: names, separated by spaces
	IrcChanWindow *pChanWin = dynamic_cast<IrcChanWindow *>(GetChildIrcWindow(msg.m_params[2]));
	
	// RPL_NAMREPLY was sent as a result of a JOIN command
	if(pChanWin && m_populatingUserList)
	{
		int numSections = msg.m_params[3].count(' ') + 1;
		for(int i = 0; i < numSections; ++i)
		{
			pChanWin->AddUser(msg.m_params[3].section(' ', i, i, QString::SectionSkipEmpty));
		}
	}
	// RPL_NAMREPLY was sent as a result of a NAMES command
	else
	{
		PrintOutput(GetNumericText(msg));
	}
}

void IrcStatusWindow::Handle366Numeric(const IrcMessage &msg)
{
	// msg.m_params[0]: my nick
	// msg.m_params[1]: channel
	// msg.m_params[2]: "End of NAMES list"
	IIrcWindow *pChanWin = GetChildIrcWindow(msg.m_params[1]);
	
	// RPL_ENDOFNAMES was sent as a result of a JOIN command
	if(pChanWin && m_populatingUserList)
	{
		m_populatingUserList = false;
	}
	// RPL_ENDOFNAMES was sent as a result of a NAMES command
	else
	{
		//PrintOutput(ConvertDataToHtml(GetNumericText(msg)));
		PrintOutput(GetNumericText(msg));
	}
}

// covers both 401 and 404 numerics
void IrcStatusWindow::Handle401Numeric(const IrcMessage &msg)
{
	// msg.m_params[0]: my nick
	// msg.m_params[1]: nick/channel
	// msg.m_params[2]: "No such nick/channel"
	IIrcWindow *pPrivWin = GetChildIrcWindow(msg.m_params[1]);
	if(pPrivWin)
	{
		pPrivWin->PrintOutput(GetNumericText(msg));
	}
	else
	{
		PrintOutput(GetNumericText(msg));
	}
}

void IrcStatusWindow::HandleInviteMsg(const IrcMessage &msg)
{
	QString textToPrint = QString("* %1 has invited you to %2")
				.arg(ParseMsgPrefix(msg.m_prefix, MsgPrefixName))
				.arg(msg.m_params[1]);
	PrintOutput(textToPrint, QColor(g_pCfgManager->GetOptionValue("colors.ini", "invite")));
}

void IrcStatusWindow::HandleJoinMsg(const IrcMessage &msg)
{
	IrcChanWindow *pChanWin = dynamic_cast<IrcChanWindow *>(GetChildIrcWindow(msg.m_params[0]));
	QString textToPrint = "* ";
	
	// if the JOIN message is of you joining the channel
	QString nickJoined = ParseMsgPrefix(msg.m_prefix, MsgPrefixName);
	if(m_pSharedService->GetNick().compare(nickJoined, Qt::CaseInsensitive) == 0)
	{
		if(pChanWin)
		{
			textToPrint += "You have rejoined ";
		}
		else
		{
			// create the channel and post the message to it
			pChanWin = new (std::nothrow) IrcChanWindow(m_pSharedService, m_pSharedConn, msg.m_params[0]);
			if(!pChanWin)
			{
				PrintError("Allocation of a new channel window failed.");
				return;
			}
			AddChanWindow(pChanWin);
			textToPrint += "You have joined ";
		}
		
		pChanWin->JoinChannel();
		textToPrint += msg.m_params[0];
		m_populatingUserList = true;
	}
	else
	{
		if(pChanWin)
		{
			textToPrint += QString("%1 (%2) has joined %3")
					.arg(ParseMsgPrefix(msg.m_prefix, MsgPrefixName))
					.arg(ParseMsgPrefix(msg.m_prefix, MsgPrefixUserAndHost))
					.arg(msg.m_params[0]);
			pChanWin->AddUser(nickJoined);
		}
	}
	
	if(pChanWin)
		pChanWin->PrintOutput(textToPrint, QColor(g_pCfgManager->GetOptionValue("colors.ini", "join")));
	else
		PrintError("Pointer to channel window is invalid. This path should not have been reached.");
}

void IrcStatusWindow::HandleKickMsg(const IrcMessage &msg)
{
    IrcChanWindow *pChanWin = dynamic_cast<IrcChanWindow *>(GetChildIrcWindow(msg.m_params[0]));
    if(!pChanWin)
        return;

	QString textToPrint;

    // if the KICK message is for you
    if(m_pSharedService->GetNick().compare(msg.m_params[1]) == 0)
    {
        pChanWin->LeaveChannel();
		textToPrint = "* You were kicked from ";
    }
    else
    {
		textToPrint = QString("* %1 was kicked from ").arg(msg.m_params[1]);
        pChanWin->RemoveUser(msg.m_params[1]);
    }

	textToPrint += QString("%1 by %2")
                .arg(msg.m_params[0])
                .arg(ParseMsgPrefix(msg.m_prefix, MsgPrefixName));

    bool hasReason = !msg.m_params[2].isEmpty();
    if(hasReason)
    {
		textToPrint += QString(" (%1)").arg(msg.m_params[2]);
    }

    QColor kickColor(g_pCfgManager->GetOptionValue("colors.ini", "kick"));
	/*
    if(hasReason)
    {
        textToPrint += QString("</font><font color=%1>)</font>").arg(kickColor.name());
    }
	*/

	pChanWin->PrintOutput(textToPrint, kickColor);
}

void IrcStatusWindow::HandleModeMsg(const IrcMessage &msg)
{
	QString textToPrint = QString("* %1 has set mode: ").arg(ParseMsgPrefix(msg.m_prefix, MsgPrefixName));

    // ignore first parameter
    for(int i = 1; i < msg.m_paramsNum; ++i)
    {
			textToPrint += msg.m_params[i];
			textToPrint += ' ';
    }

    // channel mode
    //
    // todo: fix
    if(IsChannel(msg.m_params[0]))
    {
        IrcChanWindow *pChanWin = dynamic_cast<IrcChanWindow *>(GetChildIrcWindow(msg.m_params[0]));
        if(!pChanWin)
            return;

        bool sign = true;
        QString modes = msg.m_params[1];

        for(int modesIndex = 0, paramsIndex = 2; modesIndex < modes.size(); ++modesIndex)
        {
            if(modes[modesIndex] == '+')
            {
                sign = true;
            }
            else if(modes[modesIndex] == '-')
            {
                sign = false;
            }
            else
            {
                ChanModeType type = GetChanModeType(m_pSharedService->GetChanModes(), modes[modesIndex]);
                switch(type)
                {
                    case ModeTypeA:
                    case ModeTypeB:
                    case ModeTypeC:
                    {
                        // if there's no params left then continue
                        if(paramsIndex >= msg.m_paramsNum)
                            break;

                        QChar prefix = m_pSharedService->GetPrefixRule(modes[modesIndex]);
                        if(prefix != '\0')
                        {
                            if(sign)
                                pChanWin->AddPrefixToUser(msg.m_params[paramsIndex], prefix);
                            else
                                pChanWin->RemovePrefixFromUser(msg.m_params[paramsIndex], prefix);
                        }

                        ++paramsIndex;
                        break;
                    }
                    default:
                    {

                    }
                }
            }
        }

		pChanWin->PrintOutput(textToPrint, QColor(g_pCfgManager->GetOptionValue("colors.ini", "mode")));
    }
    else	// user mode
    {
		PrintOutput(textToPrint, QColor(g_pCfgManager->GetOptionValue("colors.ini", "mode")));
    }
}

void IrcStatusWindow::HandleNickMsg(const IrcMessage &msg)
{
	// update the user's nickname if he's the one changing it
	QString oldNick = ParseMsgPrefix(msg.m_prefix, MsgPrefixName);
	bool isMyNick = (oldNick.compare(m_pSharedService->GetNick(), Qt::CaseSensitive) == 0);
	
	QString textToPrint = QString("* %1 is now known as %2")
				.arg(oldNick)
				.arg(msg.m_params[0]);
	
	QColor nickColor(g_pCfgManager->GetOptionValue("colors.ini", "nick"));
	if(isMyNick)
	{
		m_pSharedService->SetNick(msg.m_params[0]);
		PrintOutput(textToPrint, nickColor);
	}
	
	for(int i = 0; i < m_chanList.size(); ++i)
	{
		if(m_chanList[i]->HasUser(oldNick))
		{
			m_chanList[i]->ChangeUserNick(oldNick, msg.m_params[0]);
			m_chanList[i]->PrintOutput(textToPrint, nickColor);
		}
	}
	
	// will print a nick change message to the private message window
	// if we get a NICK message, which will only be if we're in
	// a channel with the person (or if the nick being changed is ours)
	for(int i = 0; i < m_privList.size(); ++i)
	{
		if(isMyNick)
		{
			m_privList[i]->PrintOutput(textToPrint, nickColor);
		}
		else
		{
			if(oldNick.compare(m_privList[i]->GetTargetNick(), Qt::CaseInsensitive) == 0)
			{
				m_privList[i]->SetTargetNick(msg.m_params[0]);
				m_privList[i]->PrintOutput(textToPrint, nickColor);
				break;
			}
		}
	}
}

void IrcStatusWindow::HandleNoticeMsg(const IrcMessage &msg)
{
	QString source;
	if(!msg.m_prefix.isEmpty())
	{
		source = ParseMsgPrefix(msg.m_prefix, MsgPrefixName);
	}
	// if m_prefix is empty, it is from the host
	else
	{
		source = m_pSharedService->GetHost();
	}
	
	QString textToPrint = QString("-%1- %2")
                            .arg(source)
                            .arg(msg.m_params[1]);

	QColor noticeColor(g_pCfgManager->GetOptionValue("colors.ini", "notice"));
	PrintOutput(textToPrint, noticeColor);
}

void IrcStatusWindow::HandlePartMsg(const IrcMessage &msg)
{
	IrcChanWindow *pChanWin = dynamic_cast<IrcChanWindow *>(GetChildIrcWindow(msg.m_params[0]));
	if(!pChanWin)
	{
		// the window received a close message and sent
		// the PART message without expecting a reply,
		// so just ignore it
		return;
	}
	
	QString textToPrint;
	
	// if the PART message is of you leaving the channel
	if(m_pSharedService->GetNick().compare(ParseMsgPrefix(msg.m_prefix, MsgPrefixName), Qt::CaseInsensitive) == 0)
	{
		textToPrint = QString("* You have left %1").arg(msg.m_params[0]);
		pChanWin->LeaveChannel();
	}
	else
	{
		textToPrint = QString("* %1 (%2) has left %3")
				.arg(ParseMsgPrefix(msg.m_prefix, MsgPrefixName))
				.arg(ParseMsgPrefix(msg.m_prefix, MsgPrefixUserAndHost))
				.arg(msg.m_params[0]);
		pChanWin->RemoveUser(ParseMsgPrefix(msg.m_prefix, MsgPrefixName));
	}
	
	// if there's a part message
	QColor partColor(g_pCfgManager->GetOptionValue("colors.ini", "part"));
	bool hasReason = (msg.m_paramsNum > 1 && !msg.m_params[1].isEmpty());
	if(hasReason)
	{
		textToPrint += QString(" (%1)").arg(msg.m_params[1]);
	}
	
	/*
	if(hasReason)
	{
		textToPrint += QString("</font><font color=%1>)</font>").arg(partColor.name());
	}
	*/
	pChanWin->PrintOutput(textToPrint, partColor);
}

void IrcStatusWindow::HandlePongMsg(const IrcMessage &msg)
{
	// the prefix is used to determine the server that
	// sends the PONG rather than the first parameter,
	// because it will always have the server in it
	//
	// example:
	//	PING hi :there
	//	:irc.server.net PONG there :hi
	QString textToPrint = QString("* PONG from %1: %2")
				.arg(msg.m_prefix)
				.arg(msg.m_params[1]);	
	PrintOutput(textToPrint);
}

void IrcStatusWindow::HandlePrivMsg(const IrcMessage &msg)
{
	CtcpRequestType requestType = GetCtcpRequestType(msg);
	QString user = ParseMsgPrefix(msg.m_prefix, MsgPrefixName);
	
	QString textToPrint;
	QColor color;
	
	if(requestType != RequestTypeInvalid)
	{
		// ACTION is /me, so handle according to that
		if(requestType == RequestTypeAction)
		{
			QString action = msg.m_params[1];
			
			// action = "\1ACTION <action>\1"
			// first 8 characters and last 1 character need to be excluded
			// so we'll take the mid, starting at index 8 and going until every
			// character but the last is included
			color.setNamedColor(g_pCfgManager->GetOptionValue("colors.ini", "action"));
			textToPrint = QString("* %1 %2")
					.arg(user)
					.arg(action.mid(8, action.size()-9));
		}
		else		// regular CTCP requests
		{
			QString replyStr;
			QString requestTypeStr;
			
			switch(requestType)
			{
				case RequestTypeVersion:
				{
					replyStr = "IMClient v0.00001";
					requestTypeStr = "VERSION";
					break;
				}
				case RequestTypeTime:
				{
					replyStr = "time for you to get a watch";
					requestTypeStr = "TIME";
					break;
				}
				case RequestTypeFinger:
				{
					replyStr = "good hustle.";
					requestTypeStr = "FINGER";
					break;
				}
				default:
				{
					requestTypeStr = "UNKNOWN";
				}
			}
			
			if(!replyStr.isEmpty())
			{
				QString textToSend = QString("NOTICE %1 :\1%2 %3\1")
                                                        .arg(user)
                                                        .arg(requestTypeStr)
                                                        .arg(replyStr);
				m_pSharedConn->Send(textToSend);
			}
			
			color.setNamedColor(g_pCfgManager->GetOptionValue("colors.ini", "ctcp"));
			textToPrint = QString("[CTCP %1 (from %2)]")
                                        .arg(requestTypeStr)
                                        .arg(user);
			PrintOutput(textToPrint, color);
			return;
		}
	}
	else	// not CTCP, so handle normally
	{
            color.setNamedColor(g_pCfgManager->GetOptionValue("colors.ini", "say"));
            textToPrint = QString("<%1> %2")
                            .arg(user)
                            .arg(msg.m_params[1]);
	}
	
	// if the target is us, then it's an actual PM
	if(m_pSharedService->GetNick().compare(msg.m_params[0], Qt::CaseInsensitive) == 0)
	{
		IrcPrivWindow *pPrivWin = dynamic_cast<IrcPrivWindow *>(GetChildIrcWindow(user));
		if(!pPrivWin)
		{
			// todo
			pPrivWin = new IrcPrivWindow(m_pSharedService, m_pSharedConn, user);
			AddPrivWindow(pPrivWin);
		}
		
		pPrivWin->PrintOutput(textToPrint, color);
	}
	else		// it's a channel
	{
		IrcChanWindow *pChanWin = dynamic_cast<IrcChanWindow *>(GetChildIrcWindow(msg.m_params[0]));
		if(pChanWin)
		{
			// todo: figure out nick prefixes later
			pChanWin->PrintOutput(textToPrint, color);
		}
	}
}

void IrcStatusWindow::HandleQuitMsg(const IrcMessage &msg)
{
	QString user = ParseMsgPrefix(msg.m_prefix, MsgPrefixName);
	QString textToPrint = QString("* %1 (%2) has quit")
							.arg(user)
							.arg(ParseMsgPrefix(msg.m_prefix, MsgPrefixUserAndHost));
	
	QColor quitColor(g_pCfgManager->GetOptionValue("colors.ini", "quit"));
	bool hasReason = (msg.m_paramsNum > 0 && !msg.m_params[0].isEmpty());
	if(hasReason)
	{
		textToPrint += QString(" (%1)").arg(msg.m_params[0]);
	}
	
	/*
	if(hasReason)
	{
		textToPrint += QString("</font><font color=%1>)</font>").arg(quitColor.name());
	}
	*/
	for(int i = 0; i < m_chanList.size(); ++i)
	{
		if(m_chanList[i]->HasUser(user))
		{
			m_chanList[i]->RemoveUser(user);
			m_chanList[i]->PrintOutput(textToPrint, quitColor);
		}
	}
	
	// will print a quit message to the private message window
	// if we get a QUIT message, which will only be if we're in
	// a channel with the person
	for(int i = 0; i < m_privList.size(); ++i)
	{
		if(user.compare(m_privList[i]->GetTargetNick(), Qt::CaseInsensitive) == 0)
		{
			m_privList[i]->PrintOutput(textToPrint, quitColor);
			break;
		}
	}
}

void IrcStatusWindow::HandleTopicMsg(const IrcMessage &msg)
{
	IIrcWindow *pChanWin = dynamic_cast<IrcChanWindow *>(GetChildIrcWindow(msg.m_params[0]));
	if(pChanWin)
	{
		QString textToPrint = QString("* %1 changes topic to: %2")
					.arg(ParseMsgPrefix(msg.m_prefix, MsgPrefixName))
					.arg(msg.m_params[1]);
		QColor topicColor(g_pCfgManager->GetOptionValue("colors.ini", "topic"));
		pChanWin->PrintOutput(textToPrint, topicColor);
		
		if(m_pManager)
		{
			QTreeWidgetItem *pItem = m_pManager->GetItemFromWindow(pChanWin);
			QString titleWithTopic = QString("%1: %2")
								.arg(pItem->text(0))
								.arg(msg.m_params[1]);
			pChanWin->SetTitle(StripCodes(titleWithTopic));
		}
	}
}

void IrcStatusWindow::HandleWallopsMsg(const IrcMessage &msg)
{
	QString textToPrint = QString("* WALLOPS from %1: %2")
				.arg(ParseMsgPrefix(msg.m_prefix, MsgPrefixName))
				.arg(msg.m_params[0]);
	PrintOutput(textToPrint);
}

// handles a disconnection fired from the Connection object
void IrcStatusWindow::HandleDisconnect()
{
	PrintOutput("* Disconnected.");
	SetTitle("Server Window");
	SetWindowName("Server Window");
}
