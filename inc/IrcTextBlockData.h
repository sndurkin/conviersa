/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QTextBlockUserData>

namespace cv { namespace irc {

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

    virtual ~IrcTextBlockData() {}

    // no substantial overhead, because QTextBlockUserData
    // already makes use of a virtual destructor
    virtual IrcTextBlockType getTextBlockType() { return TextBlockWithoutColors; }
};

class IrcColoredTextBlockData : public IrcTextBlockData
{
    int *m_pForeground;
    int *m_pReversed;

public:
    IrcColoredTextBlockData(bool isNumeric, int command);
    ~IrcColoredTextBlockData();

    IrcTextBlockType getTextBlockType() { return TextBlockWithColors; }

    void setForegroundRange(int *pForeground);
    void setReversedRange(int *pReversed);
};

} } // end namespaces