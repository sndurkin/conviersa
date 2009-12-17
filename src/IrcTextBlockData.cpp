#include "IrcTextBlockData.h"

IrcColoredTextBlockData::IrcColoredTextBlockData(bool isNumeric, int command)
	: IrcTextBlockData(isNumeric, command),
	  m_pForeground(NULL),
	  m_pReversed(NULL)
{ }

IrcColoredTextBlockData::~IrcColoredTextBlockData()
{
	if(m_pForeground) delete m_pForeground;
	if(m_pReversed) delete m_pReversed;
}
