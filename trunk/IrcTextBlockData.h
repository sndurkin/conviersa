#ifndef IRCTEXTBLOCKDATA_H
#define IRCTEXTBLOCKDATA_H

#include <QTextBlockUserData>

enum IrcTextBlockType
{
    TextBlockWithoutColors,
    TextBlockWithColors
};

class IrcTextBlockData : public QTextBlockUserData
{
protected:
    bool m_isNumeric;
    int m_command;

public:
    IrcTextBlockData(bool isNumeric, int command)
        : m_isNumeric(isNumeric),
          m_command(command)
    { }
	virtual ~IrcTextBlockData();

	// no substantial overhead, because QTextBlockUserData
	// already makes use of a virtual destructor
	virtual IrcTextBlockType GetTextBlockType() { return TextBlockWithoutColors; }
};

class IrcColoredTextBlockData : public IrcTextBlockData
{
    int *m_pForeground;
    int *m_pReversed;

public:
    IrcColoredTextBlockData(bool isNumeric, int command);
	~IrcColoredTextBlockData();

    IrcTextBlockType GetTextBlockType() { return TextBlockWithColors; }

	void SetForegroundRange(int *pForeground);
	void SetReversedRange(int *pReversed);
};

#endif
