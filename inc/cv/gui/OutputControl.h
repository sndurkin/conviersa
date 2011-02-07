/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2010 Conviersa Project
*
************************************************************************/

#pragma once

#include <QApplication>
#include <QAbstractScrollArea>
#include <QList>
#include <QLinkedList>
#include <QPoint>
#include "cv/EventManager.h"

#if defined(_MSC_VER)
    #define FORCE_INLINE __forceinline
#elif defined(__GNUG__)
    #define FORCE_INLINE __attribute__((always_inline))
#else
    #error
#endif

class QTimer;
class QMouseEvent;

namespace cv { namespace gui {

struct LinkInfo {
    int startIdx;
    int endIdx;
};

//-----------------------------------//

// event that is used for the "output" event; allows
// callbacks to add actionable links before the line
// is displayed in the control
class OutputEvent : public Event
{
    QString         m_text;
    QList<LinkInfo> m_linkInfoList;

public:
    OutputEvent(const QString &text)
      : m_text(text)
    { }
    ~OutputEvent() { }

    // accessors
    QString getText() { return m_text; }
    const QList<LinkInfo> &getLinkInfoList() { return m_linkInfoList; }

    // modifiers
    void addLinkInfo(int startIdx, int endIdx)
    {
        // do bounds checking on indices
        if(startIdx > endIdx || startIdx < 0 || endIdx >= m_text.length())
            return;

        LinkInfo linkInfo;
        linkInfo.startIdx = startIdx;
        linkInfo.endIdx = endIdx;

        // a new LinkInfo will get added ONLY
        // if it doesn't conflict with any other links
        if(!m_linkInfoList.isEmpty())
        {
            QList<LinkInfo>::iterator iter = m_linkInfoList.begin();
            while(iter != m_linkInfoList.end() && startIdx > (*iter).endIdx) { ++iter; }

            if(iter == m_linkInfoList.end())
                m_linkInfoList.append(linkInfo);
            else if(endIdx < (*iter).startIdx)
                m_linkInfoList.insert(iter, linkInfo);
        }
        else
        {
            m_linkInfoList.append(linkInfo);
        }
    }
};

//-----------------------------------//

// event that is used for the "doubleClickedLink" event
class DoubleClickLinkEvent : public Event
{
    QString         m_text;

public:
    DoubleClickLinkEvent(const QString &text)
      : m_text(text)
    { }

    // accessors
    QString getText() { return m_text; }
};

//-----------------------------------//

enum OutputColor {
    COLOR_NONE = -1,

    COLOR_CHAT_FOREGROUND,
    COLOR_CHAT_BACKGROUND,

    COLOR_CUSTOM_1,
    COLOR_CUSTOM_2,
    COLOR_CUSTOM_3,
    COLOR_CUSTOM_4,
    COLOR_CUSTOM_5,
    COLOR_CUSTOM_6,
    COLOR_CUSTOM_7,
    COLOR_CUSTOM_8,
    COLOR_CUSTOM_9,
    COLOR_CUSTOM_10,
    COLOR_CUSTOM_11,
    COLOR_CUSTOM_12,
    COLOR_CUSTOM_13,
    COLOR_CUSTOM_14,
    COLOR_CUSTOM_15,
    COLOR_CUSTOM_16,

    COLOR_CHAT_SELF,
    COLOR_HIGHLIGHT,
    COLOR_ACTION,
    COLOR_CTCP,
    COLOR_NOTICE,
    COLOR_NICK,
    COLOR_INFO,
    COLOR_INVITE,
    COLOR_JOIN,
    COLOR_PART,
    COLOR_KICK,
    COLOR_MODE,
    COLOR_QUIT,
    COLOR_TOPIC,
    COLOR_WALLOPS,
    COLOR_WHOIS,

    COLOR_DEBUG,
    COLOR_ERROR
};

//-----------------------------------//

// section of consecutive characters
// that all share the same style and color
class TextRun
{
    // used like a linked list
    TextRun *   m_nextTextRun;
    int         m_length;

    // m_textInfo (16 bits)
    //  - holds the information about the
    //    foreground color, background color,
    //    and if there is bold or underline
    // ----------------------------------------
    // | |||||| |||||| | | |
    // F      E      D C B A
    //
    // A: underline         max val: 1
    // B: bold              max val: 1
    // C: reversed          max val: 1
    // D: foreground color  max val: 3F
    // E: background color  max val: 3F
    // F: unused            max val: 1
    int      m_textInfo;

public:
    TextRun(int length = 0, int textInfo = 0)
        : m_nextTextRun(NULL),
          m_length(length),
          m_textInfo(textInfo)
    { }
    TextRun(const TextRun &tr)
        : m_nextTextRun(NULL),
          m_length(0),
          m_textInfo(tr.m_textInfo)
    { }

    // accessors
    int getLength() const { return m_length; }
    TextRun *nextTextRun() const { return m_nextTextRun; }
    bool isUnderline() const { return (m_textInfo & 0x1); }
    bool isBold() const { return (m_textInfo & 0x2); }
    bool isReversed() const { return (m_textInfo & 0x4); }
    qint8 getFgColor() const { return ((m_textInfo >> 3) & 0x3F); }
    qint8 getBgColor() const { return (m_textInfo >> 9); }
    bool hasBgColor() const { return ((m_textInfo >> 9) > 0); }

    // modifiers
    void setNextTextRun(TextRun *run) { m_nextTextRun = run; }
    void incrementLength() { ++m_length; }
    void flipUnderline() { m_textInfo ^= 0x1; }
    void flipBold() { m_textInfo ^= 0x2; }
    void flipReverse() { m_textInfo ^= 0x4; }
    void setFgColor(qint8 fgColor) { m_textInfo = (fgColor << 3) | (m_textInfo & 0x7E07); }
    void setBgColor(qint8 bgColor) { m_textInfo = (bgColor << 9) | (m_textInfo & 0x1FF); }
    void reset() { m_textInfo = 0; }
    void resetColors(qint8 defaultFgColor = 0)
    {
        m_textInfo &= 0x7;
        setFgColor(defaultFgColor);
    }
};

//-----------------------------------//

enum ChunkType
{
    UNKNOWN,
    WORD,
    WHITESPACE,
    HYPERLINK
};

class WordChunk
{
    // used like a linked list
    WordChunk * m_nextChunk;
    int         m_length;

    ChunkType   m_chunkType;
    int         m_width;

public:
    WordChunk(int length = 0, ChunkType chunkType = UNKNOWN)
        : m_nextChunk(NULL),
          m_length(length),
          m_chunkType(chunkType)
    { }

    // accessors
    int getLength() const { return m_length; }
    int getWidth() const { return m_width; }
    WordChunk *nextChunk() const { return m_nextChunk; }
    ChunkType getChunkType() const { return m_chunkType; }

    // modifiers
    void setNextChunk(WordChunk *nextChunk) { m_nextChunk = nextChunk; }
    void incrementLength() { ++m_length; }
    void setWidth(int width) { m_width = width; }
    void setChunkType(ChunkType chunkType) { m_chunkType = chunkType; }
};

//-----------------------------------//

class LinkFragment
{
    int m_x,
        m_y,
        m_width,
        m_length;

    LinkFragment *  m_nextLinkFragment;

public:
    LinkFragment(int x, int y, int width, int length)
        : m_x(x),
          m_y(y),
          m_width(width),
          m_length(length),
          m_nextLinkFragment(NULL)
    { }

    // accessors
    int x() const { return m_x; }
    int y() const { return m_y; }
    int getWidth() const { return m_width; }
    int getLength() const { return m_length; }
    LinkFragment *nextLinkFragment() { return m_nextLinkFragment; }

    // modifiers
    void setNextLinkFragment(LinkFragment *nextLinkFragment) { m_nextLinkFragment = nextLinkFragment; }
};

//-----------------------------------//

class Link
{
    int             m_startIdx,
                    m_endIdx,
                    m_width;

    Link *          m_nextLink;
    LinkFragment *  m_firstLinkFragment;

public:
    Link(int startIdx, int endIdx, int width)
        : m_startIdx(startIdx),
          m_endIdx(endIdx),
          m_width(width),
          m_nextLink(NULL),
          m_firstLinkFragment(NULL)
    { }

    // accessors
    int getStartIdx() { return m_startIdx; }
    int getEndIdx() { return m_endIdx; }
    int getWidth() { return m_width; }
    Link *nextLink() { return m_nextLink; }
    LinkFragment *firstLinkFragment() { return m_firstLinkFragment; }

    // modifiers
    void setNextLink(Link *nextLink) { m_nextLink = nextLink; }
    void setFirstLinkFragment(LinkFragment *firstLinkFragment) { m_firstLinkFragment = firstLinkFragment; }
    void destroyLinkFragments()
    {
        LinkFragment *currFragment = m_firstLinkFragment;
        LinkFragment *nextFragment;
        while(currFragment != NULL)
        {
            nextFragment = currFragment->nextLinkFragment();
            delete currFragment;
            currFragment = nextFragment;
        }
        m_firstLinkFragment = NULL;
    }
};

//-----------------------------------//

class OutputLine
{
    QString     m_text;
    WordChunk * m_firstChunk;
    TextRun *   m_firstTextRun;
    Link *      m_firstLink;

    // these variables hold information about where the
    // alternate text selection starts (if there are
    // timestamps and the user selects text while holding down
    // ctrl, then text selection will start after the
    // timestamp)
    int         m_alternateSelectionIdx;
    int         m_alternateSelectionStart;

    // this is to manage line wraps inside a word chunk
    int *       m_splits;
    int         m_numSplits;

public:

    // this is to manage text selection
    int         m_selStartIdx;
    int         m_selEndIdx;

    OutputLine()
        : m_firstChunk(NULL),
          m_firstTextRun(NULL),
          m_firstLink(NULL),
          m_numSplits(0)
    { }

    // accessors
    QString &text() { return m_text; }
    WordChunk *firstChunk() const { return m_firstChunk; }
    TextRun *firstTextRun() const { return m_firstTextRun; }
    Link *firstLink() const { return m_firstLink; }
    bool hasLinks() const { return m_firstLink != NULL; }
    bool hasTextSelection() const { return m_selStartIdx != -1; }
    int getTextSelectionStart() const { return m_selStartIdx; }
    int getTextSelectionEnd() const { return m_selEndIdx; }
    int getTextSelectionLength() const { return m_selEndIdx - m_selStartIdx + 1; }
    int getAlternateSelectionIdx() const { return m_alternateSelectionIdx; }
    int getSelectionStartIdx() const;
    int getSelectionStart() const;
    int getNumSplits() const { return m_numSplits; }
    int *getSplitsArray() const { return m_splits; }

    // modifiers
    void append(const QString &text) { m_text.append(text); }
    void append(const QChar &ch) { m_text.append(ch); }
    void setFirstTextRun(TextRun *firstTextRun) { m_firstTextRun = firstTextRun; }
    void setFirstWordChunk(WordChunk *firstChunk) { m_firstChunk = firstChunk; }
    void setFirstLink(Link *firstLink) { m_firstLink = firstLink; }
    void setSelectionRange(int startIdx, int endIdx) { m_selStartIdx = startIdx; m_selEndIdx = endIdx; }
    void unsetSelectionRange() { m_selStartIdx = -1; }
    void setAlternateSelectionIdx(int a) { m_alternateSelectionIdx = a; }
    void setAlternateSelectionStart(int a) { m_alternateSelectionStart = a; }
    void setSplitsAndClearList(QLinkedList<int> &splitsList)
    {
        if(splitsList.count() > 0)
        {
            m_numSplits = splitsList.count();
            m_splits = new int[splitsList.count()];
            int i = 0;
            while(!splitsList.isEmpty())
            {
                m_splits[i++] = splitsList.takeFirst();
            }
        }
        else
        {
            // necessary?
            m_numSplits = 0;
        }
    }
};

//-----------------------------------//

class OutputControl : public QAbstractScrollArea
{
    Q_OBJECT

    QList<OutputLine>   m_lines;
    int                 m_lastVisibleLineIdx;
    int                 m_lastVisibleWrappedLine;

    int                 m_totalWrappedLines;
    int                 m_scrollbarValue;

    bool                m_isMouseDown;
    QPoint              m_dragStartPos;
    QPoint              m_dragEndPos;

    // these two variables are used to pinpoint the current link
    // being hovered over by the mouse cursor
    int                 m_hoveredLineIdx;
    Link *              m_hoveredLink;


    // variables used instance-wide; they are used as reusable
    // blocks of memory so we don't have to call new/delete within
    // large loops
    char                m_fmBlock[sizeof(QFontMetrics)];
    char                m_evtBlock[sizeof(OutputEvent)];
    void *              m_pFM;
    void *              m_pEvt;

public:
    static const int    PADDING = 3;
    static const int    TEXT_START_POS = PADDING;
    static const int    WRAPPED_TEXT_START_POS = 25;

    // color reference array; see OutputColor enum
    static QColor       COLORS[36];

    OutputControl(QWidget *parent = NULL);
    void appendMessage(const QString &msg, OutputColor defaultMsgColor);
    void changeFont(const QFont &font);
    void refreshLinks();

protected:
    void appendLine(OutputLine &line);
    void flushOutputLines();
    void calculateLineWraps(OutputLine &currLine, QLinkedList<int> &splits, int vpWidth, QFont font);
    bool linkHitTest(int x, int y, int &lineIdx, Link *&link);
    QSize sizeHint() const;
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

public slots:
    void updateScrollbarValue(int value);
};

} } // end namespaces
