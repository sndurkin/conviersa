/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QtGui>
#include "qext.h"
#include "IrcParser.h"

namespace IrcParser
{	
	// parses the data into a structure that holds all information
	// necessary to print the message
	IrcMessage ParseData(const QString &data)
	{
		IrcMessage msg;
		
		// make an index for the sections, start at 0
		int sectionIndex = 0;
		
		// isolate the prefix, if there is one
		if(data[0] == ':')
		{
			// get the prefix, remove the ':' from it
			msg.m_prefix = data.section(' ', 0, 0, QString::SectionSkipEmpty);
			if(msg.m_prefix[0] == ':')
			{
				msg.m_prefix.remove(0, 1);
			}
			
			// make the index start at section 1
			++sectionIndex;
		}
		
		// get the command
		QString command = data.section(' ', sectionIndex, sectionIndex, QString::SectionSkipEmpty);
		++sectionIndex;
		
		// now decide which one it is
		msg.m_command = command.toInt(&msg.m_isNumeric);
		if(!msg.m_isNumeric)
		{
			// the command isn't numeric, so we still have to search for it
			if(command.compare("ERROR", Qt::CaseInsensitive) == 0)
			{
				msg.m_command = IRC_COMMAND_ERROR;
			}
			else if(command.compare("INVITE", Qt::CaseInsensitive) == 0)
			{
				msg.m_command = IRC_COMMAND_INVITE;
			}
			else if(command.compare("JOIN", Qt::CaseInsensitive) == 0)
			{
				msg.m_command = IRC_COMMAND_JOIN;
			}
			else if(command.compare("KICK", Qt::CaseInsensitive) == 0)
			{
				msg.m_command = IRC_COMMAND_KICK;
			}
			else if(command.compare("MODE", Qt::CaseInsensitive) == 0)
			{
				msg.m_command = IRC_COMMAND_MODE;
			}
			else if(command.compare("NICK", Qt::CaseInsensitive) == 0)
			{
				msg.m_command = IRC_COMMAND_NICK;
			}
			else if(command.compare("NOTICE", Qt::CaseInsensitive) == 0)
			{
				msg.m_command = IRC_COMMAND_NOTICE;
			}
			else if(command.compare("PART", Qt::CaseInsensitive) == 0)
			{
				msg.m_command = IRC_COMMAND_PART;
			}
			else if(command.compare("PING", Qt::CaseInsensitive) == 0)
			{
				msg.m_command = IRC_COMMAND_PING;
			}
			else if(command.compare("PONG", Qt::CaseInsensitive) == 0)
			{
				msg.m_command = IRC_COMMAND_PONG;
			}
			else if(command.compare("PRIVMSG", Qt::CaseInsensitive) == 0)
			{
				msg.m_command = IRC_COMMAND_PRIVMSG;
			}
			else if(command.compare("QUIT", Qt::CaseInsensitive) == 0)
			{
				msg.m_command = IRC_COMMAND_QUIT;
			}
			else if(command.compare("TOPIC", Qt::CaseInsensitive) == 0)
			{
				msg.m_command = IRC_COMMAND_TOPIC;
			}
			else if(command.compare("WALLOPS", Qt::CaseInsensitive) == 0)
			{
				msg.m_command = IRC_COMMAND_WALLOPS;
			}
			else
			{
				msg.m_command = IRC_COMMAND_UNKNOWN;
			}
		}
		
		// get the parameters
		int paramsIndex = 0;
		while(true)
		{
			// we want to divide it into parameters until we find one that starts with ':'
			// or until we find one that ends with '\r\n' (and then make that the last parameter)
			msg.m_params += data.section(' ', sectionIndex, sectionIndex, QString::SectionSkipEmpty);
			if((msg.m_params[paramsIndex])[0] == ':')
			{
				// we have to get the rest of the string, and trim it (take out the "\r\n"),
				// so that the last parameter is like all the others
				msg.m_params[paramsIndex] = data.section(' ', sectionIndex, -1, QString::SectionSkipEmpty).trimmed();
				
				// then remove the preceding colon
				msg.m_params[paramsIndex].remove(0, 1);
				
				break;
			}
			else if(msg.m_params[paramsIndex].indexOf('\n') >= 0)
			{
				// we just have to trim it
				msg.m_params[paramsIndex] = msg.m_params[paramsIndex].trimmed();
				
				break;
			}
			
			++sectionIndex;
			++paramsIndex;
		}
		
		msg.m_paramsNum = paramsIndex+1;
		
		return msg;
	}
	
	// helper function for ConvertDataToHtml()
	QString GetFontStyle(bool leadingFontTag, const QColor &defaultFg, const QColor &defaultBg, 
					const QColor &fg, const QColor &bg, bool reverse, bool underline, bool bold)
	{
		QString fontTag;
		
		if(leadingFontTag)
		{
			fontTag += "</font>";
		}
		
		fontTag += "<font style=\"";
		
		QColor actualFg;
		QColor actualBg;
		
		if(reverse)
		{
			actualFg = defaultBg;
			actualBg = defaultFg;
		}
		else
		{
			actualFg = fg;
			actualBg = bg;
		}
		
		fontTag += "color:";
		fontTag += actualFg.name();
		fontTag += ";background-color:";
		fontTag += actualBg.name();
		
		fontTag += ";text-decoration:";
		if(underline)
		{
			fontTag += "underline";
		}
		else
		{
			fontTag += "none";
		}
		
		fontTag += ";font-weight:";
		if(bold)
		{
			fontTag += "bold";
		}
		else
		{
			fontTag += "normal";
		}
		
		fontTag += "\">";
		
		return fontTag;
	}
#if 1
	// converts data received from the server, complete
	// with any control codes, to HTML-formatted text
	// ready for display
	//
	// we need the default fg and bg colors of the QTextEdit we're creating HTML for
	// so we can implement reversing colors (default is black on white)
	QString ConvertDataToHtml(const QString &text, const QColor &defaultFg/* = QColor(0, 0, 0)*/, 
						const QColor &defaultBg/* = QColor(255, 255, 255)*/)
	{
		QString textToReturn = escapeEx(text);
		
		// standard tag format: <font style="color:<fgcolor>;background-color:<bgcolor>;
		//					text-decoration:<decoration>;font-weight:<weight>">
		bool bold = false;
		bool reverse = false;
		bool underline = false;
		bool leadingFontTag = false;
		
		int charsToReplace;
		
		QColor currFg = defaultFg;
		QColor currBg = defaultBg;
		
		for(int i = 0; i < textToReturn.size(); ++i)
		{
			QString replaceWith;
			bool specialChar = true;
			
			switch(textToReturn[i].toAscii())
			{
				case 2:	// bold
				{
					charsToReplace = 1;
					bold = !bold;
					replaceWith = GetFontStyle(leadingFontTag, defaultFg, defaultBg, currFg, currBg,
										reverse, underline, bold);
					leadingFontTag = true;
					
					break;
				}
				case 15:	// causes formatting to return to normal
				{
					charsToReplace = 1;
					
					if(reverse || underline || bold)
					{
						if(reverse)
							reverse = false;
						
						if(underline)
							underline = false;
						
						if(bold)
							bold = false;
						
						replaceWith = "</font>";
						leadingFontTag = false;
					}
					
					// [17:16:03] <Chaz6> drop me a mail sometime - chaz@chaz6.com
					// [17:16:05] <Chaz6> i can help with packaging on linux
					break;
				}
				case 22:	// reverse
				{
					charsToReplace = 1;
					reverse = !reverse;
					replaceWith = GetFontStyle(leadingFontTag, defaultFg, defaultBg, currFg, currBg,
										reverse, underline, bold);
					leadingFontTag = true;
					
					break;
				}
				case 31:	// underline
				{
					charsToReplace = 1;
					underline = !underline;
					replaceWith = GetFontStyle(leadingFontTag, defaultFg, defaultBg, currFg, currBg,
										reverse, underline, bold);
					leadingFontTag = true;
					
					break;
				}
				case 3:	// color
				{
					charsToReplace = 1;
					
					// follows mIRC's method for coloring, where the
					// foreground color comes first (up to two digits),
					// and the optional background color comes last (up
					// to two digits) and they are separated by a single comma
					//
					// ex: '\3'05,02
					//
					// max length of color specification is 5
					// (four numbers and one comma)
					QString firstNum, secondNum;
					
					int tempIdx = i + 1;
					for(int j = 0; j < 2; ++j, ++tempIdx)
					{
						if(!textToReturn[tempIdx].isDigit())
						{
							if(j > 0 && textToReturn[tempIdx] == ',')
								break;
							goto end_color_spec;
						}
						++charsToReplace;
						firstNum += textToReturn[tempIdx];
					}
					
					if(textToReturn[tempIdx] != ',')
					{
						goto end_color_spec;
					}
					
					++tempIdx;
					++charsToReplace;
					for(int j = 0; j < 2; ++j, ++tempIdx)
					{
						if(!textToReturn[tempIdx].isDigit())
						{
							goto end_color_spec;
						}
						++charsToReplace;
						secondNum += textToReturn[tempIdx];
					}
					
				end_color_spec:
					
					// get the foreground color
					if(!firstNum.isEmpty())
					{
						QString colorStr = GetHtmlColor(firstNum.toInt());
						if(!colorStr.isEmpty())
						{
							currFg.setNamedColor(colorStr);
						}
					}
					else
					{
						currFg = defaultFg;
						currBg = defaultBg;
					}
					
					// get the background color
					if(!secondNum.isEmpty())
					{
						QString colorStr = GetHtmlColor(secondNum.toInt());
						if(!colorStr.isEmpty())
						{
							currBg.setNamedColor(colorStr);
						}
					}
					
					replaceWith = GetFontStyle(leadingFontTag, defaultFg, defaultBg, currFg, currBg,
										reverse, underline, bold);
					break;
				}
				default:
				{
					specialChar = false;
				}
			}
			
			if(specialChar)
			{
				textToReturn.replace(i, charsToReplace, replaceWith);
				
				// we increment i to 1 less than the size we're replacing with,
				// because the end of the loop already increments i by 1
				i += replaceWith.size()-1;
			}
		}
		
		return textToReturn;
	}
	
	// converts HTML-formatted text (sent from the input window)
	// into IRC protocol
	QString ConvertHtmlToData(const QString &text)
	{
		/*
		QTextDocument doc;
		QString data;
		QString tag;
		
		for(int i = 0; i < text.size(); ++i)
		{
			// entering an HTML tag
			if(text[i] == '<')
			{
				++i;
				tag = text.right(text.size() - i).section('>', 0, 0);
				i += 
			}
		}
		*/
            return text;
	}
#endif
	// uses data received from the server, complete
	// with any control codes, to color the text
	// in the text block that the text cursor provided is
	// pointing to
	void AddColorsToText(const QString &text, QTextCursor &cursor,
							const QColor &defaultFg/* = QColor(0, 0, 0)*/,
							const QColor &defaultBg/* = QColor(255, 255, 255)*/)
	{
		bool bold = false;
		bool underline = false;
		bool reverse = false;
		bool foregroundColor = false;
		bool backgroundColor = false;

		QColor currFg, currBg;

		QTextCharFormat currFormat = cursor.charFormat();
		currFormat.setForeground(defaultFg);

		QTextBlock currBlock = cursor.block();

		for(int i = 0; i < text.size(); ++i)
		{
			switch(text[i].toAscii())
			{
				case 2:	// bold
				{
					bold = !bold;
					QFont currFont = currFormat.font();
					currFont.setBold(bold);
					currFormat.setFont(currFont);

					break;
				}
				case 15:	// causes formatting to return to normal
				{
					currFormat.setForeground(defaultFg);
					currFormat.clearBackground();
					QFont currFont = currFormat.font();
					currFont.setBold(false);
					currFont.setUnderline(false);
					currFormat.setFont(currFont);

					bold = false;
					underline = false;
					reverse = false;
					foregroundColor = false;
					backgroundColor = false;

					// [17:16:03] <Chaz6> drop me a mail sometime - chaz@chaz6.com
					// [17:16:05] <Chaz6> i can help with packaging on linux
					break;
				}
				case 22:	// reverse
				{
					if(!reverse)
					{
						reverse = true;
						currFormat.setForeground(defaultBg);
						currFormat.setBackground(defaultFg);
					}
					else
					{
						reverse = false;

						if(foregroundColor)
							currFormat.setForeground(currFg);
						else
							currFormat.setForeground(defaultFg);

						if(backgroundColor)
							currFormat.setBackground(currBg);
						else
							currFormat.clearBackground();
					}

					break;
				}
				case 31:	// underline
				{
					underline = !underline;
					QFont currFont = currFormat.font();
					currFont.setUnderline(underline);
					currFormat.setFont(currFont);

					break;
				}
				case 3:	// color
				{
					// if the reverse control code is spotted before
					// this, then colors are ignored
					if(reverse)
						break;

					// follows mIRC's method for coloring, where the
					// foreground color comes first (up to two digits),
					// and the optional background color comes last (up
					// to two digits) and they are separated by a single comma
					//
					// ex: '\3'05,02
					//
					// max length of color specification is 5
					// (four numbers and one comma)
					QString firstNum, secondNum;

					++i;
					for(int j = 0; j < 2; ++j, ++i)
					{
						if(!text[i].isDigit())
						{
							if(j > 0 && text[i] == ',')
								break;
							goto end_color_spec;
						}
						firstNum += text[i];
					}

					if(text[i] != ',')
					{
						goto end_color_spec;
					}

					++i;
					for(int j = 0; j < 2; ++j, ++i)
					{
						if(!text[i].isDigit())
						{
							goto end_color_spec;
						}
						secondNum += text[i];
					}

				end_color_spec:
					--i;

					// get the foreground color
					if(!firstNum.isEmpty())
					{
						QString colorStr = GetHtmlColor(firstNum.toInt());
						if(!colorStr.isEmpty())
						{
							foregroundColor = true;
							currFg.setNamedColor(colorStr);
							currFormat.setForeground(currFg);
						}
					}
					else	// otherwise, the color is being terminated
					{
						foregroundColor = false;
						backgroundColor = false;
						currFormat.setForeground(defaultFg);
						currFormat.clearBackground();
					}

					// get the background color
					if(!secondNum.isEmpty())
					{
						QString colorStr = GetHtmlColor(secondNum.toInt());
						if(!colorStr.isEmpty())
						{
							backgroundColor = true;
							currBg.setNamedColor(colorStr);
							currFormat.setBackground(currBg);
						}
					}
					break;
				}
				default:
				{
					cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
					cursor.setCharFormat(currFormat);
					cursor.clearSelection();
				}
			}
		}
	}
	
	// returns the completely stripped version of the text,
	// so it doesn't contain any bold, underline, color, or other
	// control codes
	QString StripCodes(const QString &text)
	{
		QString strippedText;
		for(int i = 0; i < text.size(); ++i)
		{
			switch(text[i].toAscii())
			{
				case 1:	// CTCP stuff
				case 2:	// bold
				case 15:	// causes formatting to return to normal
				case 22:	// reverse
				case 31:	// underline
				{
					// these aren't added
					break;
				}
				case 3:	// color
				{
					// follows mIRC's method for coloring, where the
					// foreground color comes first (up to two digits),
					// and the optional background color comes last (up
					// to two digits) and they are separated by a single comma
					//
					// ex: '\3'05,02
					//
					// max length of color specification is 5
					// (four numbers and one comma)
					++i;
					for(int j = 0; j < 2; ++j, ++i)
					{
						if(!text[i].isDigit())
						{
							if(j > 0 && text[i] == ',')
								break;
							goto end_color_spec;
						}
					}
					
					if(text[i] != ',')
					{
						goto end_color_spec;
					}
					
					++i;
					for(int j = 0; j < 2; ++j, ++i)
					{
						if(!text[i].isDigit())
						{
							goto end_color_spec;
						}
					}
					
				end_color_spec:
					--i;
					break;
				}
				default:
				{
					strippedText += text[i];
				}
			}
		}
		
		return strippedText;
	}
	
	// returns the color corresponding to the 1- or 2-digit
	// number in the format "#XXXXXX" so it can be used
	// in HTML
	// 
	// if the number passed is invalid, it returns an empty string
	QString GetHtmlColor(int number)
	{
		switch(number)
		{
			case 0:	// white
			{
				return "#FFFFFF";
			}
			case 1:	// black
			{
				return "#000000";
			}
			case 2:	// navy blue
			{
				return "#000080";
			}
			case 3:	// green
			{
				return "#008000";
			}
			case 4:	// red
			{
				return "#FF0000";
			}
			case 5:	// maroon
			{
				return "#800000";
			}
			case 6:	// purple
			{
				return "#800080";
			}
			case 7:	// orange
			{
				return "#FFA500";
			}
			case 8:	// yellow
			{
				return "#FFFF00";
			}
			case 9:	// lime green
			{
				return "#00FF00";
			}
			case 10:	// teal
			{
				return "#008080";
			}
			case 11:	// cyan/aqua
			{
				return "#00FFFF";
			}
			case 12:	// blue
			{
				return "#0000FF";
			}
			case 13:	// fuschia
			{
				return "#FF00FF";
			}
			case 14:	// gray
			{
				return "#808080";
			}
			case 15:	// silver
			{
				return "#C0C0C0";
			}
			default:
			{
				return "";
			}
		}
	}
	
	// returns the type of the channel mode
	ChanModeType GetChanModeType(const QString &chanModes, const QChar &letter)
	{
		if(chanModes.section(',', 0, 0).contains(letter))
			return ModeTypeA;
		else if(chanModes.section(',', 1, 1).contains(letter))
			return ModeTypeB;
		else if(chanModes.section(',', 2, 2).contains(letter))
			return ModeTypeC;
		else if(chanModes.section(',', 3, 3).contains(letter))
			return ModeTypeD;
		
		return ModeTypeUnknown;
	}
	
	// checks for the PREFIX section in a 005 numeric, and if
	// it exists, it parses it and returns it
	QString GetPrefixRules(const QString &param)
	{
		QString prefixRules = "o@v+";
		
		// examples:
		//	PREFIX=(qaohv)~&@%+
		//	PREFIX=(ov)@+
		//
		// we have two indices for param:
		//	i starts after (
		//	j starts after )
		int i = param.indexOf('(') + 1;
		if(i <= 0)
			return prefixRules;
		
		int j = param.indexOf(')', i) + 1;
		if(j <= 0)
			return prefixRules;
		
		// iterate over the parameter
		prefixRules.clear();
		int paramSize = param.size();
		for(; param[i] != ')' && j < paramSize; ++i, ++j)
		{
			prefixRules += param[i];
			prefixRules += param[j];
		}
		
		// examples of prefixRules:
		//	q~a&o@h%v+
		//	o@v+
		return prefixRules;
	}
	
	// returns the specific CtcpRequestType of the message
	CtcpRequestType GetCtcpRequestType(const IrcMessage &msg)
	{
		// it has to be a private message
		if(msg.m_command != IRC_COMMAND_PRIVMSG)
		{
			return RequestTypeInvalid;
		}
		
		QString text = msg.m_params[1];
		if(text[0] != '\1' || text[text.size()-1] != '\1')
		{
			return RequestTypeInvalid;
		}
		
		// identify the command
		if(text.startsWith("\1action ", Qt::CaseInsensitive))
		{
			return RequestTypeAction;
		}
		else if(text.compare("\1version\1", Qt::CaseInsensitive) == 0)
		{
			return RequestTypeVersion;
		}
		else if(text.compare("\1time\1", Qt::CaseInsensitive) == 0)
		{
			return RequestTypeTime;
		}
		else if(text.compare("\1finger\1", Qt::CaseInsensitive) == 0)
		{
			return RequestTypeFinger;
		}
		
		return RequestTypeUnknown;
	}
	
	// forms the text that can be printed to the output for all numeric commands
	QString GetNumericText(const IrcMessage &msg)
	{
		QString text;
		
		// ignore the first parameter, which is the user's name
		for(int i = 1; i < msg.m_paramsNum; ++i)
		{
			text += msg.m_params[i];
			text += ' ';
		}
		
		// return the text
		return text;
	}
	
	// used to parse the msg prefix to get a specific part
	QString ParseMsgPrefix(const QString &prefix, MsgPrefixPart part)
	{
		// format of prefix: <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
		if(part == MsgPrefixName)
		{
			return prefix.section('!', 0, 0);
		}
		else if(part == MsgPrefixUserAndHost)
		{
			return prefix.section('!', 1);
		}
		else
		{
			QString userAndHost = prefix.section('!', 1);
			
			if(part == MsgPrefixUser)
			{
				return userAndHost.section('@', 0, 0);
			}
			else	// part = MsgPrefixHost
			{
				return userAndHost.section('@', 1);
			}
		}
	}
	
	// used to differentiate between a channel and a nickname
	//
	// returns true if the str is a valid name for a channel,
	// returns false otherwise
	bool IsChannel(const QString &str)
	{
		switch(str[0].toAscii())
		{
			case '#':
			case '&':
			case '+':
			case '!':
				return true;
			default:
				return false;
		}
	}
}
