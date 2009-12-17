#ifndef IRCPARSER_H
#define IRCPARSER_H

#include <QColor>
#include <QString>
#include <QStringList>
#include <QTextDocument>

namespace IrcParser
{
	enum CtcpRequestType
	{
		RequestTypeInvalid,	// not CTCP request format
		
		RequestTypeAction,
		RequestTypeVersion,
		RequestTypeTime,
		RequestTypeFinger,
		
		RequestTypeUnknown
	};	
	
	enum ChanModeType
	{
		ModeTypeUnknown,
		
		ModeTypeA,
		ModeTypeB,
		ModeTypeC,
		ModeTypeD
	};

	// used to specify a channel mode
	struct ChannelMode
	{
		bool			m_sign;
		QChar			m_mode;
		QString		m_param;
	};
	
	// used for parsing the message prefix
	enum MsgPrefixPart
	{
		MsgPrefixName,
		MsgPrefixUser,
		MsgPrefixHost,
		MsgPrefixUserAndHost
	};
	
	// lists all the possible non-numeric irc commands
	enum
	{
		IRC_COMMAND_UNKNOWN,
		
		// alphabetical order
		IRC_COMMAND_ERROR,
		IRC_COMMAND_INVITE,
		IRC_COMMAND_JOIN,
		IRC_COMMAND_KICK,
		IRC_COMMAND_MODE,
		IRC_COMMAND_NICK,
		IRC_COMMAND_NOTICE,
		IRC_COMMAND_PART,
		IRC_COMMAND_PING,
		IRC_COMMAND_PONG,
		IRC_COMMAND_PRIVMSG,
		IRC_COMMAND_QUIT,
		IRC_COMMAND_TOPIC,
		IRC_COMMAND_WALLOPS
	};
	
	struct IrcMessage
	{
		QString 		m_prefix;
		bool			m_isNumeric;
		int			m_command;
		int			m_paramsNum;
		QStringList		m_params;
	};
	
	// parses the data into a structure that holds all information
	// necessary to print the message
	IrcMessage ParseData(const QString &data);
	
	// converts data received from the server, complete
	// with any control codes, to HTML-formatted text
	// ready for display
	//
	// we need the default fg and bg colors of the QTextEdit we're creating HTML for
	// so we can implement reversing colors (default is black on white)
	QString ConvertDataToHtml(const QString &text, const QColor &defaultFg = QColor(0, 0, 0), 
						const QColor &defaultBg = QColor(255, 255, 255));
	
	// converts HTML-formatted text (sent from the input window)
	// into IRC protocol
	QString ConvertHtmlToData(const QString &text);

	// uses data received from the server, complete
	// with any control codes, to color the text
	// in the text block that the text cursor provided is
	// pointing to
	void AddColorsToText(const QString &text, QTextCursor &cursor,
							const QColor &defaultFg = QColor(0, 0, 0),
							const QColor &defaultBg = QColor(255, 255, 255));
	
	// returns the completely stripped version of the text,
	// so it doesn't contain any bold, underline, color, or other
	// control codes
	QString StripCodes(const QString &text);
	
	// returns the color corresponding to the 1- or 2-digit
	// number in the format "#XXXXXX" so it can be used
	// in HTML
	// 
	// if the number passed is invalid, it returns an empty string
	QString GetHtmlColor(int number);
	
	// returns the type of the channel mode
	ChanModeType GetChanModeType(const QString &chanModes, const QChar &letter);
	
	// checks for the PREFIX section in a 005 numeric, and if
	// it exists, it parses it and returns it
	QString GetPrefixRules(const QString &param);
	
	// returns the specific CtcpRequestType of the message
	CtcpRequestType GetCtcpRequestType(const IrcMessage &msg);
	
	// returns the text for all numeric commands
	QString GetNumericText(const IrcMessage &msg);
	
	// used to parse a part of the msg prefix
	QString ParseMsgPrefix(const QString &prefix, MsgPrefixPart part);
	
	// decide between using the given text and "you" in a message
	// todo: change name, it sucks
	QString GetUser(const QString &nick, const QString &user);
	
	// used to differentiate between a channel and a nickname
	//
	// returns true if the str is a valid name for a channel,
	// returns false otherwise
	bool IsChannel(const QString &str);
}

#endif
