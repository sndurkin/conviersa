/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2010 Conviersa Project
*
************************************************************************/

#include <QApplication>
#include <QClipboard>
#include <QPainter>
#include <QScrollBar>
#include <QPalette>
#include <QLinkedList>
#include <QMouseEvent>
#include <QDateTime>
#include <QDebug>
#include <QStyle>
#include <QStyleOption>
#include <QWindowsVistaStyle>
#include "cv/ConfigManager.h"
#include "cv/gui/OutputControl.h"

namespace cv { namespace gui {

QString OutputControl::COLOR_TO_CONFIG_MAP[COLOR_NUM] = {
    "output.color.say",             // COLOR_CHAT_FOREGROUND
    "output.color.background",      // COLOR_CHAT_BACKGROUND

    "output.color.custom1",         // COLOR_CUSTOM_X
    "output.color.custom2",
    "output.color.custom3",
    "output.color.custom4",
    "output.color.custom5",
    "output.color.custom6",
    "output.color.custom7",
    "output.color.custom8",
    "output.color.custom9",
    "output.color.custom10",
    "output.color.custom11",
    "output.color.custom12",
    "output.color.custom13",
    "output.color.custom14",
    "output.color.custom15",
    "output.color.custom16",

    "output.color.say.self",        // COLOR_CHAT_SELF
    "output.color.highlight",       // COLOR_HIGHLIGHT
    "output.color.action",          // COLOR_ACTION
    "output.color.ctcp",            // COLOR_CTCP
    "output.color.notice",          // COLOR_NOTICE
    "output.color.nick",            // COLOR_NICK
    "output.color.info",            // COLOR_INFO
    "output.color.invite",          // COLOR_INVITE
    "output.color.join",            // COLOR_JOIN
    "output.color.part",            // COLOR_PART
    "output.color.kick",            // COLOR_KICK
    "output.color.mode",            // COLOR_MODE
    "output.color.quit",            // COLOR_QUIT
    "output.color.topic",           // COLOR_TOPIC
    "output.color.wallops",         // COLOR_WALLOPS
    "output.color.whois",           // COLOR_WHOIS

    "output.color.debug",           // COLOR_DEBUG
    "output.color.error"            // COLOR_ERROR
};

int OutputLine::getSelectionStartIdx() const
{
    if(QApplication::keyboardModifiers() & Qt::ControlModifier)
        return m_alternateSelectionIdx;
    return 0;
}

//-----------------------------------//

int OutputLine::getSelectionStart() const
{
    if(QApplication::keyboardModifiers() & Qt::ControlModifier)
        return m_alternateSelectionStart;
    return OutputControl::TEXT_START_POS;
}

//-----------------------------------//

OutputControl::OutputControl(QWidget *parent/*= NULL*/)
    : QAbstractScrollArea(parent),
      m_isMouseDown(false),
      m_lastVisibleLineIdx(-1),
      m_totalWrappedLines(0),
      m_hoveredLineIdx(-1),
      m_hoveredLink(NULL)
{
    setFont(QFont("Consolas", 10));
    viewport()->setMouseTracking(true);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    verticalScrollBar()->setSingleStep(1);
    QObject::connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateScrollbarValue(int)));

    // set pointers for the blocks of memory
    m_pFM = m_fmBlock;
    m_pEvt = m_evtBlock;

    setupColors();
    g_pEvtManager->hookGlobalEvent("configChanged", STRING_EVENT, MakeDelegate(this, &OutputControl::onConfigChanged));
}

OutputControl::~OutputControl()
{
    g_pEvtManager->unhookGlobalEvent("configChanged", STRING_EVENT, MakeDelegate(this, &OutputControl::onConfigChanged));
}

//-----------------------------------//

QSize OutputControl::sizeHint() const
{
    return QSize(800, 500);
}

//-----------------------------------//

void OutputControl::setupColorConfig(QMap<QString, ConfigOption> &defOptions)
{
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CHAT_FOREGROUND], ConfigOption("#000000", CONFIG_TYPE_COLOR));
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CHAT_BACKGROUND], ConfigOption("#ffffff", CONFIG_TYPE_COLOR));

    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CUSTOM_1], ConfigOption("#ffffff", CONFIG_TYPE_COLOR));    // white
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CUSTOM_2], ConfigOption("#000000", CONFIG_TYPE_COLOR));    // black
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CUSTOM_3], ConfigOption("#000080", CONFIG_TYPE_COLOR));    // navy
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CUSTOM_4], ConfigOption("#008000", CONFIG_TYPE_COLOR));    // green
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CUSTOM_5], ConfigOption("#ff0000", CONFIG_TYPE_COLOR));    // red
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CUSTOM_6], ConfigOption("#800000", CONFIG_TYPE_COLOR));    // maroon
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CUSTOM_7], ConfigOption("#800080", CONFIG_TYPE_COLOR));    // purple
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CUSTOM_8], ConfigOption("#ffa500", CONFIG_TYPE_COLOR));    // orange
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CUSTOM_9], ConfigOption("#ffff00", CONFIG_TYPE_COLOR));    // yellow
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CUSTOM_10], ConfigOption("#00ff00", CONFIG_TYPE_COLOR));   // lime
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CUSTOM_11], ConfigOption("#008080", CONFIG_TYPE_COLOR));   // teal
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CUSTOM_12], ConfigOption("#00ffff", CONFIG_TYPE_COLOR));   // cyan
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CUSTOM_13], ConfigOption("#0000ff", CONFIG_TYPE_COLOR));   // blue
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CUSTOM_14], ConfigOption("#ff00ff", CONFIG_TYPE_COLOR));   // magenta
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CUSTOM_15], ConfigOption("#808080", CONFIG_TYPE_COLOR));   // gray
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CUSTOM_16], ConfigOption("#c0c0c0", CONFIG_TYPE_COLOR));   // light gray

    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CHAT_SELF], ConfigOption("#000000", CONFIG_TYPE_COLOR));
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_HIGHLIGHT], ConfigOption("#ff0000", CONFIG_TYPE_COLOR));
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_ACTION], ConfigOption("#0000cc", CONFIG_TYPE_COLOR));
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_CTCP], ConfigOption("#d80000", CONFIG_TYPE_COLOR));
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_NOTICE], ConfigOption("#b80000", CONFIG_TYPE_COLOR));
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_NICK], ConfigOption("#808080", CONFIG_TYPE_COLOR));
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_INFO], ConfigOption("#000000", CONFIG_TYPE_COLOR));
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_INVITE], ConfigOption("#808080", CONFIG_TYPE_COLOR));
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_JOIN], ConfigOption("#006600", CONFIG_TYPE_COLOR));
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_PART], ConfigOption("#006600", CONFIG_TYPE_COLOR));
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_KICK], ConfigOption("#000000", CONFIG_TYPE_COLOR));
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_MODE], ConfigOption("#808080", CONFIG_TYPE_COLOR));
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_QUIT], ConfigOption("#006600", CONFIG_TYPE_COLOR));
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_TOPIC], ConfigOption("#006600", CONFIG_TYPE_COLOR));
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_WALLOPS], ConfigOption("#b80000", CONFIG_TYPE_COLOR));
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_WHOIS], ConfigOption("#000000", CONFIG_TYPE_COLOR));

    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_DEBUG], ConfigOption("#8b0000", CONFIG_TYPE_COLOR));
    defOptions.insert(COLOR_TO_CONFIG_MAP[COLOR_ERROR], ConfigOption("#8b0000", CONFIG_TYPE_COLOR));
}

//-----------------------------------//

void OutputControl::setupColors()
{
    // TODO: possible optimization would be to create a color manager
    //       which shares QColor instances within the array
    for(int i = 0; i < COLOR_NUM; ++i)
        m_colorsArr[i] = GET_COLOR(COLOR_TO_CONFIG_MAP[i]);

    // the background color is changed a little differently; it uses CSS
    setStyleSheet(QString("cv--gui--OutputControl { background-color: %1 }").arg(GET_STRING(COLOR_TO_CONFIG_MAP[COLOR_CHAT_BACKGROUND])));
}

//-----------------------------------//

void OutputControl::onConfigChanged(Event *pEvent)
{
    ConfigEvent *pCfgEvent = DCAST(ConfigEvent, pEvent);
    QString optName = pCfgEvent->getName();

    for(int i = 0; i < COLOR_NUM; ++i)
    {
        if(optName.compare(COLOR_TO_CONFIG_MAP[i], Qt::CaseInsensitive) == 0)
        {
            // if it's the background color, we also need to set it in the QSS
            if(i == COLOR_CHAT_BACKGROUND)
                setStyleSheet(QString("cv--gui--OutputControl { background-color: %1 }").arg(pCfgEvent->getString()));

            m_colorsArr[i] = QColor(pCfgEvent->getColor());
            viewport()->update();
            break;
        }
    }
}

//-----------------------------------//

void OutputControl::appendMessage(const QString &msg, OutputColor defaultMsgColor)
{
    OutputLine line;

    TextRun *currTextRun = new TextRun();
    currTextRun->setFgColor(defaultMsgColor);
    line.setFirstTextRun(currTextRun);

    QString msgToDisplay;
    if(GET_BOOL("timestamp"))
    {
        msgToDisplay = QString("%1 ").arg(QDateTime::currentDateTime().toString(GET_STRING("timestamp.format")));
        line.setAlternateSelectionIdx(msgToDisplay.length());
        msgToDisplay += msg;
    }
    else
    {
        msgToDisplay = msg;
        line.setAlternateSelectionIdx(0);
    }

    // iterate through msg, determining the various TextRuns
    for(int i = 0, msgLen = msgToDisplay.length(); i < msgLen; ++i)
    {
        switch(msgToDisplay[i].toLatin1())
        {
            case 2:     // bold
            {
                TextRun *nextTextRun = new TextRun(*currTextRun);
                nextTextRun->flipBold();
                currTextRun->setNextTextRun(nextTextRun);
                currTextRun = nextTextRun;
                break;
            }
            case 15:    // causes formatting to return to normal
            {
                TextRun *nextTextRun = new TextRun(*currTextRun);
                nextTextRun->reset();
                nextTextRun->setFgColor(defaultMsgColor);
                currTextRun->setNextTextRun(nextTextRun);
                currTextRun = nextTextRun;
                break;
            }
            case 22:    // reverse
            {
                TextRun *nextTextRun = new TextRun(*currTextRun);
                nextTextRun->flipReverse();
                currTextRun->setNextTextRun(nextTextRun);
                currTextRun = nextTextRun;
                break;
            }
            case 31:    // underline
            {
                TextRun *nextTextRun = new TextRun(*currTextRun);
                nextTextRun->flipUnderline();
                currTextRun->setNextTextRun(nextTextRun);
                currTextRun = nextTextRun;
                break;
            }
            case 3:     // color
            {
                // if the reverse control code is spotted before
                // this, then colors are ignored
                if(currTextRun->isReversed())
                    break;

                TextRun *nextTextRun = new TextRun(*currTextRun);

                // follows mIRC's method for coloring, where the
                // foreground color comes first (up to two digits),
                // and the optional background color comes last (up
                // to two digits) and they are separated by a single comma
                //
                // ex: '\3'[05[,02]]
                //
                // min length of color specification is 0
                // max length of color specification is 5
                // (four numbers and one comma)
                QString firstNum, secondNum;

                ++i;
                for(int j = 0; j < 2; ++j, ++i)
                {
                    if(i >= msgLen)
                        goto end_color_spec;
                    if(!msgToDisplay[i].isDigit())
                    {
                        if(j > 0 && msgToDisplay[i] == ',')
                            break;
                        goto end_color_spec;
                    }
                    firstNum += msgToDisplay[i];
                }

                if(i >= msgLen || msgToDisplay[i] != ',')
                    goto end_color_spec;

                ++i;
                for(int j = 0; j < 2; ++j, ++i)
                {
                    if(i >= msgLen || !QChar(msgToDisplay[i]).isDigit())
                        goto end_color_spec;
                    secondNum += msgToDisplay[i];
                }

            end_color_spec:
                if(i < msgLen)
                    --i;

                // get the foreground color
                if(!firstNum.isEmpty())
                    nextTextRun->setFgColor(firstNum.toInt()+2);
                // otherwise, the color is being terminated
                else
                    nextTextRun->resetColors(defaultMsgColor);

                // get the background color
                if(!secondNum.isEmpty())
                    nextTextRun->setBgColor(secondNum.toInt()+2);

                currTextRun->setNextTextRun(nextTextRun);
                currTextRun = nextTextRun;
                break;
            }
            default:
            {
                currTextRun->incrementLength();
                line.append(msgToDisplay[i]);
            }
        } // switch
    } // for

    // iterate through to split into appropriate word chunks
    WordChunk *currChunk = new WordChunk();
    line.setFirstWordChunk(currChunk);
    currTextRun = line.firstTextRun();
    int currTextRunIdx = 0,
        currWidth = 0;
    QFont font = this->font();
    font.setBold(currTextRun->isBold());
    QFontMetrics *fm = new(m_pFM) QFontMetrics(font);
    QString &text = line.text();
    for(int i = 0, textLen = text.length(); i < textLen; ++i)
    {
        // update the current text run
        for(; currTextRun != NULL; currTextRun = currTextRun->nextTextRun())
        {
            if(currTextRunIdx + currTextRun->getLength() > i)
                break;
            else
                currTextRunIdx += currTextRun->getLength();
        }

        // update the QFontMetrics object if necessary
        if(font.bold() != currTextRun->isBold())
        {
            font.setBold(currTextRun->isBold());
            fm->~QFontMetrics();
            fm = new(m_pFM) QFontMetrics(font);
        }

        if(text[i] == '\t' || text[i] == ' ')
        {
            currChunk->incrementLength();
            currWidth += fm->width(text[i]);

            if(currChunk->getChunkType() == UNKNOWN)
            {
                currChunk->setChunkType(WHITESPACE);
            }
            else if(currChunk->getChunkType() == WORD)
            {
                currChunk->setWidth(currWidth);
                currWidth = 0;

                WordChunk *nextChunk = new WordChunk(0, WHITESPACE);
                currChunk->setNextChunk(nextChunk);
                currChunk = nextChunk;
            }
        }
        else
        {
            if(currChunk->getChunkType() == UNKNOWN)
            {
                currChunk->setChunkType(WORD);
            }
            else if(currChunk->getChunkType() == WHITESPACE)
            {
                if(currChunk->getLength() == 0)
                {
                    currChunk->setChunkType(WORD);
                }
                else
                {
                    currChunk->setWidth(currWidth);
                    currWidth = 0;

                    WordChunk *nextChunk = new WordChunk(0, WORD);
                    currChunk->setNextChunk(nextChunk);
                    currChunk = nextChunk;
                }
            }

            currWidth += fm->width(text[i]);
            currChunk->incrementLength();
        }
    }

    // set the width of the last chunk in the line
    currChunk->setWidth(currWidth);

    // fire onOutput event for callbacks to use for adding links
    // to the text
    OutputEvent *pEvent = new(m_pEvt) OutputEvent(line.text());
    g_pEvtManager->fireEvent("output", this, pEvent);

    // iterate through the OutputEvent to add the links given the
    // LinkInfo
    QList<LinkInfo>::const_iterator iter;
    font = this->font();
    fm->~QFontMetrics();
    fm = new(m_pFM) QFontMetrics(font);
    Link *prevLink = NULL;
    currTextRunIdx = 0;
    currTextRun = line.firstTextRun();
    for(iter = pEvent->getLinkInfoList().begin(); iter != pEvent->getLinkInfoList().end(); ++iter)
    {
        int width = 0;
        LinkInfo linkInfo = *iter;
        for(int i = linkInfo.startIdx; i <= linkInfo.endIdx; ++i)
        {
            // update the current text run
            for(; currTextRun != NULL; currTextRun = currTextRun->nextTextRun())
            {
                if(currTextRunIdx + currTextRun->getLength() > i)
                    break;
                else
                    currTextRunIdx += currTextRun->getLength();
            }

            // update the QFontMetrics object if necessary
            if(font.bold() != currTextRun->isBold())
            {
                font.setBold(currTextRun->isBold());
                fm->~QFontMetrics();
                fm = new(m_pFM) QFontMetrics(font);
            }

            // calculate the width of the next character
            width += fm->width(line.text()[i]);
        }

        Link *link = new Link(linkInfo.startIdx, linkInfo.endIdx, width);
        if(prevLink == NULL)
            line.setFirstLink(link);
        else
            prevLink->setNextLink(link);
        prevLink = link;
    }
    fm->~QFontMetrics();
    pEvent->~OutputEvent();

    appendLine(line);
}

//-----------------------------------//

void OutputControl::appendLine(OutputLine &line)
{
    // if we're appending the first line
    if(m_lastVisibleLineIdx < 0)
    {
        m_lastVisibleLineIdx = 0;
        m_lastVisibleWrappedLine = 0;
        verticalScrollBar()->setRange(1, 1);
        verticalScrollBar()->setPageStep(50);
    }

    // calculate line wraps for this line
    QLinkedList<int> splits;
    calculateLineWraps(line, splits, viewport()->width(), this->font());
    m_totalWrappedLines += line.getNumSplits() + 1;

    m_lines.append(line);

    // initiate a repaint of the viewport
    flushOutputLines();
}

//-----------------------------------//

void OutputControl::flushOutputLines()
{
    // for the very first line, there is no scrollbar, so the code
    // below doesn't work; in this case, we just create a paint event
    // manually
    if(m_lines.size() == 1)
    {
        viewport()->update();
    }
    // in this case, there is more than 1 line, so setting the scrollbar
    // value will cause a repaint
    else
    {
        bool atBottom = verticalScrollBar()->maximum() == verticalScrollBar()->value();
        verticalScrollBar()->setRange(1, m_totalWrappedLines);
        if(atBottom)
        {
            int scrollbarValue = m_totalWrappedLines + m_lastVisibleWrappedLine;
            verticalScrollBar()->setValue(scrollbarValue);
        }
    }
}

//-----------------------------------//

// changes the font for the OutputControl by resetting
// the widths of the chunks in every line and creating
// a dummy resize event to force a recalculation of all
// line wraps
void OutputControl::changeFont(const QFont &newFont)
{
    setFont(newFont);
    viewport()->setFont(newFont);

    QFont font = newFont;
    for(int i = 0; i < m_lines.size(); ++i)
    {
        OutputLine &currLine = m_lines[i];

        WordChunk *currChunk = currLine.firstChunk();
        TextRun *currTextRun = currLine.firstTextRun();
        int currTextRunIdx = 0,
            currWidth = 0;
        int chkStart = 0;
        font.setBold(currTextRun->isBold());
        QFontMetrics *fm = new(m_pFM) QFontMetrics(font);
        QString &text = currLine.text();
        for(int j = 0, k = 0; j < text.length(); ++j, ++k)
        {
            // update the current text run
            for(; currTextRun != NULL; currTextRun = currTextRun->nextTextRun())
            {
                if(currTextRunIdx + currTextRun->getLength() > j)
                    break;
                else
                    currTextRunIdx += currTextRun->getLength();
            }

            // update the QFontMetrics object if necessary
            if(font.bold() != currTextRun->isBold())
            {
                font.setBold(currTextRun->isBold());
                fm->~QFontMetrics();
                fm = new(m_pFM) QFontMetrics(font);
            }

            if(k == currChunk->getLength())
            {
                currChunk->setWidth(currWidth);
                currWidth = k = 0;
                chkStart = j;
                currChunk = currChunk->nextChunk();
            }
            currWidth += fm->width(text[j]);
        }

        // set the width of the last chunk in the line
        currChunk->setWidth(currWidth);

        fm->~QFontMetrics();
    }

    // create dummy resize event
    QSize oldSize(width() - 1, height());
    resizeEvent(new QResizeEvent(size(), oldSize));
}

//-----------------------------------//

void OutputControl::resizeEvent(QResizeEvent *event)
{
    if(event->oldSize().width() != event->size().width())
    {
        QFont font = this->font();
        int vpWidth = viewport()->width();

        // 1) TEXT WRAPPING
        //     - use word chunks and text runs to calculate all the line splits, and
        //       then set the scrollbar value based on how many wrapped lines there are
        QLinkedList<int> splits;
        m_totalWrappedLines = 0;
        int scrollbarValue;
        for(int i = 0, numLines = m_lines.length(); i < numLines; ++i)
        {
            OutputLine &currLine = m_lines[i];
            int oldSplitsNum = currLine.getNumSplits();

            calculateLineWraps(currLine, splits, vpWidth, font);

            if(i == m_lastVisibleLineIdx)
            {
                // if the last visible line has been resized to have fewer
                // wrapped lines than before (control has been widened) and
                // it was at the last wrapped line, make it hug the new last
                // wrapped line
                bool moveScrollbarToBottom = m_lastVisibleWrappedLine > currLine.getNumSplits();

                // if the scrollbar was at the bottom before shrinking the width,
                // then keep it there
                moveScrollbarToBottom |= oldSplitsNum < currLine.getNumSplits() && m_lastVisibleWrappedLine == oldSplitsNum;

                if(moveScrollbarToBottom)
                    m_lastVisibleWrappedLine = currLine.getNumSplits();
                scrollbarValue = m_totalWrappedLines + m_lastVisibleWrappedLine + 1;
            }

            m_totalWrappedLines += currLine.getNumSplits() + 1;
        }

        // reset the scrollbar range and scrollbar value
        verticalScrollBar()->setRange(1, m_totalWrappedLines);
        verticalScrollBar()->setValue(scrollbarValue);
    }
}

//-----------------------------------//

void OutputControl::calculateLineWraps(OutputLine &currLine, QLinkedList<int> &splits, int vpWidth, QFont font)
{
    int currWidth = TEXT_START_POS,
        currTextIdx = 0,
        currTextRunIdx = 0;
    TextRun *currTextRun = currLine.firstTextRun();
    WordChunk *currChunk,
              *firstChunk = currLine.firstChunk();

    font.setBold(currTextRun->isBold());
    QFontMetrics *fm = new(m_pFM) QFontMetrics(font);

    for(currChunk = firstChunk; currChunk != NULL; currChunk = currChunk->nextChunk())
    {
        int availableWidth = vpWidth - currWidth;

        #define WRAP_CHUNK() \
            /* find all the splits required to make this chunk fit */ \
            int fragmentIdx = 0; \
            while(fragmentIdx < currChunk->getLength()) \
            { \
                availableWidth = vpWidth - currWidth; \
                \
                /* update to the proper text run (if needed) */ \
                while(currTextRunIdx + currTextRun->getLength() <= currTextIdx + fragmentIdx) \
                { \
                    currTextRunIdx += currTextRun->getLength(); \
                    currTextRun = currTextRun->nextTextRun(); \
                } \
                \
                /* we only change the QFontMetrics object if necessary */ \
                if(font.bold() != currTextRun->isBold()) \
                { \
                    font.setBold(currTextRun->isBold()); \
                    fm->~QFontMetrics(); \
                    fm = new(m_pFM) QFontMetrics(font); \
                } \
                \
                /* determine if width of next character fits or not */ \
                int nextCharWidth = fm->width(currLine.text()[currTextIdx + fragmentIdx]); \
                if(nextCharWidth <= availableWidth || currWidth == WRAPPED_TEXT_START_POS) \
                { \
                    /* it does, so add it into the width */ \
                    currWidth += nextCharWidth; \
                    ++fragmentIdx; \
                } \
                else \
                { \
                    /* it doesn't, so split it */ \
                    splits.append(currTextIdx + fragmentIdx); \
                    currWidth = WRAPPED_TEXT_START_POS; \
                } \
            }

        if(currChunk->getWidth() <= availableWidth)
        {
            currWidth += currChunk->getWidth();
        }
        else
        {
            bool chunkNeedsWrapping =
                    currChunk->getChunkType() == HYPERLINK
                 || currChunk->getChunkType() == WHITESPACE
                 || currChunk == firstChunk
                 || currChunk->getLength() > 24;

            // case 1: chunk needs to be wrapped
            if(chunkNeedsWrapping)
            {
                WRAP_CHUNK();
            }
            // case 2: we move word chunk to the next line
            else
            {
                splits.append(currTextIdx);
                currWidth = WRAPPED_TEXT_START_POS;
                availableWidth = vpWidth - currWidth;
                if(currChunk->getWidth() <= availableWidth)
                {
                    // it fits
                    currWidth += currChunk->getWidth();
                }
                else
                {
                    WRAP_CHUNK();
                }
            }
        }

        currTextIdx += currChunk->getLength();
    }

    // set the splits on the line
    currLine.setSplitsAndClearList(splits);

    // if this line has a timestamp, then we want to set
    // the alternate text selection start
    if(currLine.getAlternateSelectionIdx() > 0)
    {
        int x = TEXT_START_POS;
        currTextIdx = currTextRunIdx = 0;
        currTextRun = currLine.firstTextRun();
        font.setBold(currTextRun->isBold());

        while(currTextIdx < currLine.getAlternateSelectionIdx())
        {
            // find the text run for our current text idx
            while(currTextRunIdx + currTextRun->getLength() <= currTextIdx)
            {
                currTextRunIdx += currTextRun->getLength();
                currTextRun = currTextRun->nextTextRun();
            }

            // update font metrics object if necessary
            if(currTextRun != NULL && font.bold() != currTextRun->isBold())
            {
                font.setBold(currTextRun->isBold());
                fm->~QFontMetrics();
                fm = new(m_pFM) QFontMetrics(font);
            }

            // if the text run fits before the alternate selection idx
            if(currTextRunIdx + currTextRun->getLength() <= currLine.getAlternateSelectionIdx())
            {
                // then we want to find the width of all the text
                // and add it into the x-coord
                int len = currTextRun->getLength() - (currTextIdx - currTextRunIdx);
                x += fm->width(currLine.text().mid(currTextIdx, len));

                // move to the next text run
                currTextRunIdx += currTextRun->getLength();
                currTextIdx = currTextRunIdx;
                currTextRun = currTextRun->nextTextRun();
            }
            // if the text run doesn't fit
            else
            {
                // then we find the width up to just before alternate selection idx
                int len = currLine.getAlternateSelectionIdx() - currTextIdx;
                x += fm->width(currLine.text().mid(currTextIdx, len));
                break;
            }
        }

        currLine.setAlternateSelectionStart(x);
    }
    else
        currLine.setAlternateSelectionStart(TEXT_START_POS);

    // now that we have the indices where the OutputLine is split,
    // we can use them to calculate the link fragments for each link
    if(currLine.hasLinks())
    {
        Link *currLink = currLine.firstLink();
        int *splitsArray = currLine.getSplitsArray();
        int numSplits = currLine.getNumSplits();
        currTextRunIdx = 0;
        currTextRun = currLine.firstTextRun();
        int j,
            x,
            nextLineIdx,
            wrappedLineNum;

        // if there are no line splits
        if(numSplits == 0)
        {
            x = TEXT_START_POS;
            currTextIdx = 0;
            nextLineIdx = currLine.text().length();
            wrappedLineNum = 0;
        }
        // line has at least 1 split
        else
        {
            for(j = 0; j < numSplits; ++j)
            {
                if(currLink->getStartIdx() < splitsArray[j])
                {
                    if(j == 0)
                    {
                        x = TEXT_START_POS;
                        currTextIdx = 0;
                    }
                    else
                    {
                        x = WRAPPED_TEXT_START_POS;
                        currTextIdx = splitsArray[j-1];
                    }

                    wrappedLineNum = j;
                    nextLineIdx = splitsArray[j];
                    break;
                }
            }

            // the start idx is on the last line
            if(j == numSplits)
            {
                x = WRAPPED_TEXT_START_POS;
                currTextIdx = splitsArray[j-1];
                nextLineIdx = currLine.text().length();
                wrappedLineNum = j;
            }
        }

        font.setBold(currTextRun->isBold());
        fm->~QFontMetrics();
        fm = new(m_pFM) QFontMetrics(font);

        LinkFragment *prevFragment = NULL;
        currLink->destroyLinkFragments();
        while(currLink != NULL)
        {
            // find the text run for our current text idx
            while(currTextRunIdx + currTextRun->getLength() <= currTextIdx)
            {
                currTextRunIdx += currTextRun->getLength();
                currTextRun = currTextRun->nextTextRun();
            }

            // update font metrics object if necessary
            if(font.bold() != currTextRun->isBold())
            {
                font.setBold(currTextRun->isBold());
                fm->~QFontMetrics();
                fm = new(m_pFM) QFontMetrics(font);
            }

            while(currTextIdx < currLink->getStartIdx())
            {
                // if the entire text run fits before the start index of the link
                if(currTextRunIdx + currTextRun->getLength() <= currLink->getStartIdx())
                {
                    // then we want to find the width of all the text
                    // and add it into the x-coord
                    int len = currTextRun->getLength() - (currTextIdx - currTextRunIdx);
                    x += fm->width(currLine.text().mid(currTextIdx, len));

                    // move to the next text run
                    currTextRunIdx += currTextRun->getLength();
                    currTextIdx = currTextRunIdx;
                    currTextRun = currTextRun->nextTextRun();

                    // we only change the QFontMetrics object if necessary
                    if(currTextRun != NULL && font.bold() != currTextRun->isBold())
                    {
                        font.setBold(currTextRun->isBold());
                        fm->~QFontMetrics();
                        fm = new(m_pFM) QFontMetrics(font);
                    }
                }
                // if the text run doesn't fit
                else
                {
                    // then we find the width up to just before the start idx
                    int len = currLink->getStartIdx() - currTextIdx;
                    x += fm->width(currLine.text().mid(currTextIdx, len));
                    currTextIdx = currLink->getStartIdx();
                }
            }

            // find the y-coord of the link fragment
            int y = wrappedLineNum * fm->lineSpacing();

            // save the fragment start and end
            int fragmentStart = currTextIdx,
                afterFragmentEnd = (currLink->getEndIdx() < nextLineIdx) ? currLink->getEndIdx() + 1
                                                                         : nextLineIdx;

            // determine the width of the link fragment
            int width = 0;
            while(currTextIdx < afterFragmentEnd)
            {
                // if the entire text run fits before the end index of the link
                if(currTextRunIdx + currTextRun->getLength() <= afterFragmentEnd)
                {
                    // then we want to find the width of all the text
                    // and add it into the x-coord
                    int len = currTextRun->getLength() - (currTextIdx - currTextRunIdx);
                    width += fm->width(currLine.text().mid(currTextIdx, len));

                    // move to the next text run
                    currTextRunIdx += currTextRun->getLength();
                    currTextIdx = currTextRunIdx;
                    currTextRun = currTextRun->nextTextRun();

                    // we only change the QFontMetrics object if necessary
                    if(currTextRun != NULL && font.bold() != currTextRun->isBold())
                    {
                        font.setBold(currTextRun->isBold());
                        fm->~QFontMetrics();
                        fm = new(m_pFM) QFontMetrics(font);
                    }
                }
                // if the text run doesn't fit...
                else
                {
                    // ...then we find the width up to just before the end idx
                    int len = afterFragmentEnd - currTextIdx;
                    width += fm->width(currLine.text().mid(currTextIdx, len));
                    currTextIdx = afterFragmentEnd;
                }
            }

            LinkFragment *newFragment = new LinkFragment(x, y, width, afterFragmentEnd - fragmentStart);
            if(prevFragment == NULL)
                currLink->setFirstLinkFragment(newFragment);
            else
                prevFragment->setNextLinkFragment(newFragment);
            prevFragment = newFragment;

            // if the link wraps to a new line...
            if(currLink->getEndIdx() >= nextLineIdx)
            {
                // ...then we want to set the fragment and start over on the new line
                x = WRAPPED_TEXT_START_POS;
                ++wrappedLineNum;
                nextLineIdx = (wrappedLineNum == numSplits) ? currLine.text().length()
                                                            : splitsArray[wrappedLineNum];
            }
            else
            {
                // we've evaluated all the fragments in the link, so move
                // onto the next link, if it exists
                currLink = currLink->nextLink();
                if(currLink != NULL)
                {
                    prevFragment = NULL;
                    currLink->destroyLinkFragments();

                    // if the next link is on a new line, then we want to reset all the values
                    if(currLink->getStartIdx() >= nextLineIdx)
                    {
                        // we start at 1 because at this point, it can't be the first wrapped line
                        for(j = 1; j < numSplits; ++j)
                        {
                            if(currLink->getStartIdx() < splitsArray[j])
                            {
                                nextLineIdx = splitsArray[j];
                                wrappedLineNum = j;
                                break;
                            }
                        }

                        if(j == numSplits)
                        {
                            nextLineIdx = currLine.text().length();
                            wrappedLineNum = j;
                        }

                        x = WRAPPED_TEXT_START_POS;
                        currTextIdx = splitsArray[wrappedLineNum - 1];
                    }
                    else
                    {
                        // otherwise, update the x value with the width of the fragment
                        x += width;
                    }
                }
            }
        }
    }

    // clean up the memory for the font metrics object
    fm->~QFontMetrics();
}

//-----------------------------------//

void OutputControl::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_isMouseDown = true;
        m_dragStartPos = event->pos();
    }
}

//-----------------------------------//

void OutputControl::mouseMoveEvent(QMouseEvent *event)
{
    if(m_isMouseDown)
    {
        m_dragEndPos = event->pos();

        QFontMetrics fontMetrics(this->font());
        int currHeight = viewport()->height() - PADDING;
        int lineSpacing = fontMetrics.lineSpacing();

        // these three variables help hold information about which drag
        // position we're currently looking at
        //    - foundFirstPos indicates whether the first drag position
        //      has already been found within another OutputLine
        //    - foundBothPos obviously is for when we're done with text selection
        //    - firstPosIndex is used to store the first index found within
        //      an OutputLine, if the two drag points are both inside the same one
        bool foundFirstPos = false, foundBothPos = false;
        int firstPosIndex = -1;

        // these two variables are used for deciding whether a given drag position
        // is really within a line; we are using a basic comparison of whether or not
        // the position's y coord is greater than the OutputLine's y coord, and because
        // this can be true for every OutputLine above the correct one, we also
        // check to make sure that we still need the position
        bool foundStartPos = false, foundEndPos = false;

        // 2) TEXT SELECTION
        //     - if the user is dragging the mouse, search all visible
        //       OutputLines for the two mouse coordinates (drag start
        //       and drag end) and calculate the start and end indices
        //       for the selected text
        for(int i = m_lastVisibleLineIdx; i >= 0 && currHeight > 0; --i)
        {
            OutputLine &currLine = m_lines[i];
            int *splitsArray = currLine.getSplitsArray();
            int numSplits = currLine.getNumSplits();

            // if we're looking at the last visible line, then
            // start at whatever wrapped line within this OutputLine
            // that the scrollbar is currently at (unless it's no longer
            // there)
            //  - currHeight will always be between two wrapped lines; initially,
            //    for example, currHeight will be right before the entire
            //    OutputLine
            if(i == m_lastVisibleLineIdx && m_lastVisibleWrappedLine <= numSplits)
                currHeight -= lineSpacing * (m_lastVisibleWrappedLine + 1);
            else
                currHeight -= lineSpacing * (numSplits + 1);

            // three cases:
            //  1) both points are within the current OutputLine
            //  2) only one point is within the current OutputLine
            //  3) neither of the points are within the current OutputLine
            bool dragStartWithinLine = !foundStartPos && m_dragStartPos.y() >= currHeight;
            bool dragEndWithinLine = !foundEndPos && m_dragEndPos.y() >= currHeight;
            if(dragStartWithinLine && dragEndWithinLine)
            {
                // 1) both points are within the current OutputLine
                int initialHeight = currHeight;
                for(int j = 0; j <= numSplits; ++j)
                {
                    // three cases:
                    //   1) both points are within this wrapped line
                    //   2) only one point is within this wrapped line
                    //   3) neither point is within the wrapped line (do nothing)
                    currHeight += lineSpacing;
                    bool dragStartInWrappedLine = !foundStartPos && m_dragStartPos.y() < currHeight;
                    bool dragEndInWrappedLine = !foundEndPos && m_dragEndPos.y() < currHeight;
                    if(dragStartInWrappedLine && dragEndInWrappedLine)
                    {
                        // we'll copy more code from below, but things are slightly
                        // different here because we're looking for both points
                        // at the same time
                        //
                        // two cases:
                        //   1) both points are between the same characters
                        //   2) the two points are between different characters
                        int currX, currTextIdx, charWidth, halfCharWidth;
                        if(j == 0)
                        {
                            currX = currLine.getSelectionStart();
                            currTextIdx = currLine.getSelectionStartIdx();
                        }
                        else
                        {
                            currX = WRAPPED_TEXT_START_POS;
                            currTextIdx = splitsArray[j - 1];
                        }

                        // store information for text run
                        TextRun *currTextRun = currLine.firstTextRun();
                        int currTextRunIdx = 0;
                        QFont font(this->font());
                        QFontMetrics *fm = new(m_pFM) QFontMetrics(font);

                        // iterate through all the characters in the line
                        int lastTextIdx = (j < numSplits) ? splitsArray[j] - 1
                                                          : currLine.text().length() - 1;
                        for(; currTextIdx <= lastTextIdx; ++currTextIdx)
                        {
                            // at this point, currX should be at a position between two characters,
                            // and currTextIdx should hold the index of the second character
                            //
                            // make sure we're up-to-date on our TextRun (which is used to
                            // ensure we are grabbing the correct width of the next character)
                            for(; currTextRun != NULL; currTextRun = currTextRun->nextTextRun())
                            {
                                if(currTextRunIdx + currTextRun->getLength() > currTextIdx)
                                    break;
                                else
                                    currTextRunIdx += currTextRun->getLength();
                            }

                            // update the QFontMetrics object if necessary
                            if(font.bold() != currTextRun->isBold())
                            {
                                font.setBold(currTextRun->isBold());
                                fm->~QFontMetrics();
                                fm = new(m_pFM) QFontMetrics(font);
                            }

                            // calculate the width of the next character
                            charWidth = fm->width(currLine.text()[currTextIdx]);
                            halfCharWidth = charWidth >> 1;

                            bool dragStartBetweenChars = !foundStartPos && m_dragStartPos.x() < currX + halfCharWidth;
                            bool dragEndBetweenChars = !foundEndPos && m_dragEndPos.x() < currX + halfCharWidth;

                            if(dragStartBetweenChars && dragEndBetweenChars)
                            {
                                currLine.unsetSelectionRange();
                                foundBothPos = true;
                                break;
                            }
                            else if(dragStartBetweenChars || dragEndBetweenChars)
                            {
                                if(dragStartBetweenChars)
                                    foundStartPos = true;
                                else
                                    foundEndPos = true;

                                if(firstPosIndex >= currLine.getSelectionStartIdx())
                                {
                                    currLine.setSelectionRange(firstPosIndex, currTextIdx - 1);
                                    foundBothPos = true;
                                    break;
                                }
                                else
                                    firstPosIndex = currTextIdx;
                            }

                            currX += charWidth;
                        }

                        if(!foundBothPos)
                        {
                            if(firstPosIndex >= currLine.getSelectionStartIdx())
                            {
                                currLine.setSelectionRange(firstPosIndex, currTextIdx - 1);
                                foundBothPos = true;
                            }
                            else
                            {
                                // if firstPosIndex is still -1, then neither drag position
                                // has been found, so they're both at the end of this line
                                currLine.unsetSelectionRange();
                            }
                        }

                        fm->~QFontMetrics();
                    }
                    else if(dragStartInWrappedLine || dragEndInWrappedLine)
                    {
                        // 2) only one point is within this wrapped line
                        QPoint &dragPos = dragStartInWrappedLine ? m_dragStartPos
                                                                 : m_dragEndPos;
                        if(dragStartInWrappedLine)
                            foundStartPos = true;
                        else
                            foundEndPos = true;

                        // now we need to find the exact characters it's between
                        //
                        // two cases:
                        //   1) it's between one of the characters within
                        //      the wrapped line (or before the first character)
                        //   2) it's after the last character of the wrapped line
                        int currX, currTextIdx, charWidth, halfCharWidth;
                        if(j == 0)
                        {
                            currX = currLine.getSelectionStart();
                            currTextIdx = currLine.getSelectionStartIdx();
                        }
                        else
                        {
                            currX = WRAPPED_TEXT_START_POS;
                            currTextIdx = splitsArray[j - 1];
                        }

                        // store information for text run
                        TextRun *currTextRun = currLine.firstTextRun();
                        int currTextRunIdx = 0;
                        QFont font(this->font());
                        QFontMetrics *fm = new(m_pFM) QFontMetrics(font);

                        // this boolean allows us to keep track of whether
                        // we found the drag position within the line (so
                        // we can check after the loop for the second case)
                        bool foundPosInLoop = false;

                        // iterate through all the characters in the line
                        int lastTextIdx = (j < numSplits) ? splitsArray[j] - 1
                                                          : currLine.text().length() - 1;
                        for(; currTextIdx <= lastTextIdx; ++currTextIdx)
                        {
                            // at this point, currX should be at a position between two characters,
                            // and currTextIdx should hold the index of the second character
                            //
                            // make sure we're up-to-date on our TextRun (which is used to
                            // ensure we are grabbing the correct width of the next character)
                            for(; currTextRun != NULL; currTextRun = currTextRun->nextTextRun())
                            {
                                if(currTextRunIdx + currTextRun->getLength() > currTextIdx)
                                    break;
                                else
                                    currTextRunIdx += currTextRun->getLength();
                            }

                            // update the QFontMetrics object if necessary
                            if(font.bold() != currTextRun->isBold())
                            {
                                font.setBold(currTextRun->isBold());
                                fm->~QFontMetrics();
                                fm = new(m_pFM) QFontMetrics(font);
                            }

                            // calculate the width of the next character
                            charWidth = fm->width(currLine.text()[currTextIdx]);
                            halfCharWidth = charWidth >> 1;

                            if(dragPos.x() < currX + halfCharWidth)
                            {
                                // two cases:
                                //   1) foundFirstPos is true, which means we have already
                                //      found the first drag position in this OutputLine,
                                //      which means this drag position is after the first one
                                //   2) this is the first drag position, which means
                                //      it's before the second drag position
                                if(firstPosIndex >= currLine.getSelectionStartIdx())
                                {
                                    currLine.setSelectionRange(firstPosIndex, currTextIdx - 1);
                                    foundBothPos = true;
                                }
                                else
                                {
                                    firstPosIndex = currTextIdx;
                                }

                                foundPosInLoop = true;
                                break;
                            }
                            else
                            {
                                currX += charWidth;
                            }
                        }

                        // 2) it's after the last character of the wrapped line
                        if(!foundPosInLoop)
                        {
                            if(firstPosIndex >= currLine.getSelectionStartIdx())
                            {
                                currLine.setSelectionRange(firstPosIndex, currTextIdx - 1);
                                foundBothPos = true;
                            }
                            else
                            {
                                firstPosIndex = currTextIdx;
                            }
                        }

                        fm->~QFontMetrics();
                    }

                    if(foundBothPos)
                        break;
                }

                currHeight = initialHeight;
                foundStartPos = foundEndPos = true;
            }
            else if(dragStartWithinLine || dragEndWithinLine)
            {
                // 2) only one point is within the current OutputLine
                QPoint &dragPos = dragStartWithinLine ? m_dragStartPos
                                                      : m_dragEndPos;
                if(dragStartWithinLine)
                    foundStartPos = true;
                else
                    foundEndPos = true;

                int initialHeight = currHeight;
                for(int j = 0; j <= numSplits; ++j)
                {
                    // is the dragPos within the current wrapped line?
                    currHeight += lineSpacing;
                    if(dragPos.y() < currHeight)
                    {
                        // then we need to find the exact characters it's between
                        //
                        // two cases:
                        //   1) it's between one of the characters within
                        //      the wrapped line (or before the first character)
                        //   2) it's after the last character of the wrapped line
                        int currX, currTextIdx, charWidth, halfCharWidth;
                        if(j == 0)
                        {
                            currX = currLine.getSelectionStart();
                            currTextIdx = currLine.getSelectionStartIdx();
                        }
                        else
                        {
                            currX = WRAPPED_TEXT_START_POS;
                            currTextIdx = splitsArray[j - 1];
                        }

                        // store information for text run
                        TextRun *currTextRun = currLine.firstTextRun();
                        int currTextRunIdx = 0;
                        QFont font(this->font());
                        QFontMetrics *fm = new(m_pFM) QFontMetrics(font);

                        // this boolean allows us to keep track of whether
                        // we found the drag position within the line (so
                        // we can check after the loop for the second case)
                        bool foundPosInLoop = false;

                        // iterate through all the characters in the line
                        int lastTextIdx = (j < numSplits) ? splitsArray[j] - 1
                                                          : currLine.text().length() - 1;
                        for(; currTextIdx <= lastTextIdx; ++currTextIdx)
                        {
                            // at this point, currX should be at a position between two characters,
                            // and currTextIdx should hold the index of the second character
                            //
                            // make sure we're up-to-date on our TextRun (which is used to
                            // ensure we are grabbing the correct width of the next character)
                            for(; currTextRun != NULL; currTextRun = currTextRun->nextTextRun())
                            {
                                if(currTextRunIdx + currTextRun->getLength() > currTextIdx)
                                    break;
                                else
                                    currTextRunIdx += currTextRun->getLength();
                            }

                            // update the QFontMetrics object if necessary
                            if(font.bold() != currTextRun->isBold())
                            {
                                font.setBold(currTextRun->isBold());
                                fm->~QFontMetrics();
                                fm = new(m_pFM) QFontMetrics(font);
                            }

                            // calculate the width of the next character
                            charWidth = fm->width(currLine.text()[currTextIdx]);
                            halfCharWidth = charWidth >> 1;

                            if(dragPos.x() < currX + halfCharWidth)
                            {
                                // two cases:
                                //   1) foundFirstPos is true, which means we have already
                                //      found the first drag position in another OutputLine,
                                //      which means this drag position is above the first one
                                //   2) this is the first drag position, which means
                                //      it's below the second drag position
                                if(foundFirstPos)
                                {
                                    currLine.setSelectionRange(currTextIdx, currLine.text().length() - 1);
                                    foundBothPos = true;
                                }
                                else
                                {
                                    if(currTextIdx > 0)
                                        currLine.setSelectionRange(currLine.getSelectionStartIdx(), currTextIdx - 1);
                                    else
                                        currLine.unsetSelectionRange();
                                    foundFirstPos = true;
                                }

                                foundPosInLoop = true;
                                break;
                            }
                            else
                            {
                                currX += charWidth;
                            }
                        }

                        // 2b) it's after the last character of the wrapped line
                        if(!foundPosInLoop)
                        {
                            if(foundFirstPos)
                            {
                                if(lastTextIdx == currLine.text().length() - 1)
                                    currLine.unsetSelectionRange();
                                else
                                    currLine.setSelectionRange(lastTextIdx + 1, currLine.text().length() - 1);
                                foundBothPos = true;
                            }
                            else
                            {
                                currLine.setSelectionRange(currLine.getSelectionStartIdx(), lastTextIdx);
                                foundFirstPos = true;
                            }
                        }

                        fm->~QFontMetrics();
                        break;
                    }
                }

                currHeight = initialHeight;
            }
            else
            {
                // 3) neither of the points are within the current OutputLine
                //      - we're going to move on to the next OutputLine,
                //        but if we have already found the first drag position
                //        then we need to select the entire line
                if(foundFirstPos && !foundBothPos)
                    currLine.setSelectionRange(currLine.getSelectionStartIdx(), currLine.text().length() - 1);
                else
                    currLine.unsetSelectionRange();
            }
        }
    }
    // if mouse is not down
    else
    {
        int lineIdx;
        Link *link;
        bool hitTestResult = linkHitTest(event->pos().x(), event->pos().y(), lineIdx, link);
        if(hitTestResult)
        {
            if(m_hoveredLink == link)
            {
                // nothing changed, so just exit
                return;
            }

            viewport()->setCursor(Qt::PointingHandCursor);
            m_hoveredLineIdx = lineIdx;
            m_hoveredLink = link;
        }
        else
        {
            if(m_hoveredLink == NULL)
            {
                // nothing changed, so just exit
                return;
            }

            m_hoveredLineIdx = -1;
            m_hoveredLink = NULL;
            viewport()->setCursor(Qt::ArrowCursor);
        }
    }

    viewport()->update();
}

//-----------------------------------//

bool OutputControl::linkHitTest(int x, int y, int &lineIdx, Link *&link)
{
    // find which OutputLine the cursor is within
    QFontMetrics *fm = new(m_pFM) QFontMetrics(this->font());
    int currHeight = viewport()->height() - PADDING;
    int lineSpacing = fm->lineSpacing();
    for(int i = m_lastVisibleLineIdx; i >= 0 && currHeight > 0; --i)
    {
        OutputLine &currLine = m_lines[i];
        int numSplits = currLine.getNumSplits();

        // determine the current height of the OutputLine
        if(i == m_lastVisibleLineIdx && m_lastVisibleWrappedLine <= numSplits)
            currHeight -= lineSpacing * (m_lastVisibleWrappedLine + 1);
        else
            currHeight -= lineSpacing * (numSplits + 1);

        // check if the mouse position is inside the current OutputLine
        if(y >= currHeight)
        {
            if(currLine.hasLinks())
            {
                // mouseY is an offset value from the beginning
                // of the OutputLine
                int mouseX = x;
                int mouseY = y - currHeight;

                // iterate through each link
                Link *currLink = currLine.firstLink();
                while(currLink != NULL)
                {
                    // iterate through each fragment in the link, checking if the mouse
                    // position is inside any of them
                    LinkFragment *currFragment = currLink->firstLinkFragment();
                    while(currFragment != NULL)
                    {
                        // check y coord
                        if(currFragment->y() <= mouseY && mouseY < currFragment->y() + lineSpacing)
                        {
                            // check x coord
                            if(currFragment->x() <= mouseX && mouseX < currFragment->x() + currFragment->getWidth())
                            {
                                lineIdx = i;
                                link = currLink;
                                fm->~QFontMetrics();
                                return true;
                            }
                        }

                        currFragment = currFragment->nextLinkFragment();
                    }

                    currLink = currLink->nextLink();
                }
            }

            break;
        }
    }

    fm->~QFontMetrics();
    return false;
}

//-----------------------------------//

void OutputControl::mouseReleaseEvent(QMouseEvent *)
{
    m_isMouseDown = false;
    viewport()->update();

    // iterate through the visible OutputLines to find the current selection
    // (if any), and copy the text to clipboard
    bool inSelection = false;
    int i = m_lastVisibleLineIdx,
        currHeight = viewport()->height() - 2 - PADDING,
        lineSpacing = QFontMetrics(font()).lineSpacing();
    for(; i >= 0 && currHeight > 0; --i)
    {
        OutputLine &currLine = m_lines[i];

        int numSplits = currLine.getNumSplits();
        if(i == m_lastVisibleLineIdx && m_lastVisibleWrappedLine <= numSplits)
            currHeight -= lineSpacing * m_lastVisibleWrappedLine;
        else
            currHeight -= lineSpacing * numSplits;

        if(currLine.hasTextSelection() && !inSelection)
        {
            inSelection = true;
        }
        else if(inSelection  && !currLine.hasTextSelection())
        {
            ++i;
            break;
        }
    }

    if(inSelection)
    {
        if(i < 0) i = 0;

        // iterate through all OutputLines which are selected and append
        // them to create the total selected text
        QString selectedText;
        while(i <= m_lastVisibleLineIdx && m_lines[i].hasTextSelection())
        {
            selectedText += m_lines[i].text().mid(m_lines[i].getTextSelectionStart(),
                                                  m_lines[i].getTextSelectionLength());
            selectedText += '\n';
            ++i;
        }

        // remove last appended newline character
        selectedText.remove(selectedText.length() - 1, 1);

        QApplication::clipboard()->setText(selectedText);
    }
}

//-----------------------------------//

void OutputControl::mouseDoubleClickEvent(QMouseEvent *event)
{
    int lineIdx;
    Link *link;
    if(linkHitTest(event->pos().x(), event->pos().y(), lineIdx, link))
    {
        QString text = m_lines[lineIdx].text().mid(link->getStartIdx(), link->getEndIdx() - link->getStartIdx() + 1);
        DoubleClickLinkEvent *pEvent = new DoubleClickLinkEvent(text);
        g_pEvtManager->fireEvent("doubleClickedLink", this, pEvent);
        delete pEvent;
    }
}

//-----------------------------------//

// this algorithm is complex, so it was broken down into parts:
//
//    1) TEXT WRAPPING
//        - calculate all the line splits and reset the scrollbar
//        - this doesn't need to be done on every paint,
//          only when the width has changed since last paint
//        - each line split is an index of the first character
//          on the next wrapped line
//
//    2) TEXT SELECTION
//        - if the user is dragging the mouse, search all visible
//          OutputLines for the two mouse coordinates (drag start
//          and drag end) and calculate the start and end indices
//          for the selected text
//
//    3) TEXT DRAWING
//        - start from the last visible OutputLine (determined in part 1)
//          and draw each one until we reach the top of the viewport
//
void OutputControl::paintEvent(QPaintEvent *)
{
    int vpWidth = viewport()->width(),
        vpHeight = viewport()->height();
    QPainter painter(viewport());

    int lineSpacing = painter.fontMetrics().lineSpacing(),
        ascent = painter.fontMetrics().ascent();

    // draw viewport background
    //painter.fillRect(0, 0, vpWidth, vpHeight, m_colorsArr[COLOR_CHAT_BACKGROUND]);

    // 3) TEXT DRAWING
    //     - start from the last visible OutputLine (determined in part 1)
    //       and draw each one until we reach the top of the viewport
    //     - currHeight will always hold the value of the baseline of the line
    //       being drawn
    int currHeight = viewport()->height() - PADDING - painter.fontMetrics().descent();
    for(int i = m_lastVisibleLineIdx; i >= 0 && currHeight > 0; --i)
    {
        OutputLine &currLine = m_lines[i];

        // initialize values
        int currWidth = TEXT_START_POS,
            currTextIdx = 0;
        TextRun *currTextRun;

        int *splitsArray = currLine.getSplitsArray();
        int numSplits = currLine.getNumSplits();
        int splitIdx = 0;

        // if we're drawing the last visible line, then
        // start at whatever wrapped line within this OutputLine
        // that the scrollbar is currently at (unless it's no longer there)
        if(i == m_lastVisibleLineIdx && m_lastVisibleWrappedLine <= numSplits)
            currHeight -= lineSpacing * m_lastVisibleWrappedLine;
        else
            currHeight -= lineSpacing * numSplits;
        int initialHeight = currHeight;

        // use text runs and line splits to draw the visible portion of text
        for(currTextRun = currLine.firstTextRun(); currTextRun != NULL; currTextRun = currTextRun->nextTextRun())
        {
            QFont font = painter.font();
            font.setBold(currTextRun->isBold());
            font.setUnderline(currTextRun->isUnderline());
            painter.setFont(font);
            QFontMetrics fm = painter.fontMetrics();

            // we have to draw long text runs (text runs that span a split index)
            // in fragments
            int fragmentOffsetIdx = 0, fragmentLength;
            int currTextRunLength = currTextRun->getLength();
            while(true)
            {
                int fragmentStartIdx = currTextIdx + fragmentOffsetIdx;

                // calculate how much of the current text run to draw at this height
                fragmentLength = currTextRunLength - fragmentOffsetIdx;
                if(splitIdx < numSplits && (currTextIdx + currTextRunLength) > splitsArray[splitIdx])
                {
                    fragmentLength = splitsArray[splitIdx] - (currTextIdx + fragmentOffsetIdx);
                }

                // five cases for text selection:
                //   1) text selection starts inside a text fragment
                //        [unselected text] [selected text]
                //   2) text selection ends inside a text fragment
                //        [selected text] [unselected text]
                //   3) text selection starts and ends inside a text fragment
                //        [unselected text] [selected text] [unselected text]
                //   4) text selection spans the entire text fragment
                //        [selected text]
                //   5) no text selection at all
                //        [unselected text]
                //
                // we can generalize these five cases to three fragments
                // of varying length, depending on the case:
                //      [unselected text] [selected text] [unselected text]
                int unselectedFragment1Length = 0,
                    selectedFragmentLength = 0,
                    unselectedFragment2Length = fragmentLength;
                if(m_isMouseDown && currLine.hasTextSelection())
                {
                    // cache some variables
                    int fragmentEndIdx = fragmentStartIdx + fragmentLength - 1;
                    int textSelectionStart = currLine.getTextSelectionStart(),
                        textSelectionEnd = currLine.getTextSelectionEnd();

                    // case 1
                    if(fragmentStartIdx < textSelectionStart
                    && textSelectionStart <= fragmentEndIdx
                    && fragmentEndIdx < textSelectionEnd)
                    {
                        unselectedFragment1Length = textSelectionStart - fragmentStartIdx;
                        selectedFragmentLength = fragmentLength - unselectedFragment1Length;
                        unselectedFragment2Length = 0;
                    }
                    // case 2
                    else if(textSelectionStart < fragmentStartIdx
                         && fragmentStartIdx <= textSelectionEnd
                         && textSelectionEnd < fragmentEndIdx)
                    {
                        selectedFragmentLength = textSelectionEnd - fragmentStartIdx + 1;
                        unselectedFragment2Length = fragmentLength - selectedFragmentLength;
                    }
                    // case 3
                    else if(fragmentStartIdx <= textSelectionStart
                         && textSelectionEnd <= fragmentEndIdx)
                    {
                        unselectedFragment1Length = textSelectionStart - fragmentStartIdx;
                        unselectedFragment2Length = fragmentEndIdx - textSelectionEnd;
                        selectedFragmentLength = fragmentLength - (unselectedFragment1Length + unselectedFragment2Length);
                    }
                    // case 4
                    else if(textSelectionStart <= fragmentStartIdx
                         && fragmentEndIdx <= textSelectionEnd)
                    {
                        selectedFragmentLength = fragmentLength;
                        unselectedFragment2Length = 0;
                    }
                }

                // we're using macros here mostly for code reuse but also to make things look
                // a lot cleaner
                #define DRAW_UNSELECTED_TEXT(previousLen, len) \
                    if(len > 0) \
                    { \
                        QString textFragment = currLine.text().mid(fragmentStartIdx + previousLen, len); \
                        int fragmentWidth = fm.width(textFragment); \
                        if(currTextRun->isReversed()) \
                        { \
                            painter.fillRect(currWidth, currHeight - ascent, fragmentWidth, lineSpacing, m_colorsArr[COLOR_CHAT_FOREGROUND]); \
                            painter.setPen(m_colorsArr[COLOR_CHAT_BACKGROUND]); \
                        } \
                        else \
                        { \
                            if(currTextRun->hasBgColor()) \
                            { \
                                painter.fillRect(currWidth, currHeight - ascent, fragmentWidth, lineSpacing, m_colorsArr[currTextRun->getBgColor()]); \
                            } \
                            painter.setPen(m_colorsArr[currTextRun->getFgColor()]); \
                        } \
                        \
                        painter.drawText(currWidth, currHeight, textFragment); \
                        currWidth += fragmentWidth; \
                    }

                #define DRAW_SELECTED_TEXT(previousLen, len) \
                    if(len > 0) \
                    { \
                        QString textFragment = currLine.text().mid(fragmentStartIdx + previousLen, len); \
                        int fragmentWidth = fm.width(textFragment); \
                        painter.fillRect(currWidth, currHeight - ascent, fragmentWidth, lineSpacing, m_colorsArr[COLOR_CHAT_FOREGROUND]); \
                        painter.setPen(m_colorsArr[COLOR_CHAT_BACKGROUND]); \
                        painter.drawText(currWidth, currHeight, textFragment); \
                        currWidth += fragmentWidth; \
                    }

                // draw the text: [unselected] [selected] [unselected]
                int totalLength = 0;
                DRAW_UNSELECTED_TEXT(totalLength, unselectedFragment1Length)
                totalLength += unselectedFragment1Length;
                DRAW_SELECTED_TEXT(totalLength, selectedFragmentLength)
                totalLength += selectedFragmentLength;
                DRAW_UNSELECTED_TEXT(totalLength, unselectedFragment2Length)

                // was there a split inside the text run?
                if(fragmentOffsetIdx + fragmentLength < currTextRunLength)
                {
                    // then we still need to draw the rest of it
                    ++splitIdx;
                    currWidth = WRAPPED_TEXT_START_POS;
                    currHeight += lineSpacing;
                    fragmentOffsetIdx += fragmentLength;
                }
                // if not...
                else
                {
                    // ...then move on to the next text run
                    currTextIdx += currTextRunLength;
                    break;
                }
            }
        }

        // after we've drawn the entire OutputLine, we need to check to see if one
        // of its Links is being hovered over; if so, we have to redraw it with underline
        if(i == m_hoveredLineIdx)
        {
            // using macros again for unselected/selected text
            #define DRAW_UNSELECTED_LINK_TEXT(index, len, x, y) \
                if(len > 0) \
                { \
                    QString textFragment = currLine.text().mid(index, len); \
                    int fragmentWidth = fm.width(textFragment); \
                    if(currTextRun->isReversed()) \
                        painter.setPen(m_colorsArr[COLOR_CHAT_BACKGROUND]); \
                    else \
                        painter.setPen(m_colorsArr[currTextRun->getFgColor()]); \
                    painter.drawText(x, y, textFragment); \
                    x += fragmentWidth; \
                }

            #define DRAW_SELECTED_LINK_TEXT(index, len, x, y) \
                if(len > 0) \
                { \
                    QString textFragment = currLine.text().mid(index, len); \
                    int fragmentWidth = fm.width(textFragment); \
                    painter.setPen(m_colorsArr[COLOR_CHAT_BACKGROUND]); \
                    painter.drawText(x, y, textFragment); \
                    x += fragmentWidth; \
                }

            // draw each LinkFragment individually
            int currTextRunIdx = 0;
            currTextIdx = m_hoveredLink->getStartIdx();
            currTextRun = currLine.firstTextRun();
            LinkFragment *currFragment = m_hoveredLink->firstLinkFragment();
            while(currFragment != NULL)
            {
                int afterFragmentEnd = currTextIdx + currFragment->getLength();
                while(currTextIdx < afterFragmentEnd)
                {
                    // make sure the text run is up-to-date
                    for(; currTextRun != NULL; currTextRun = currTextRun->nextTextRun())
                    {
                        if(currTextRunIdx + currTextRun->getLength() > currTextIdx)
                            break;
                        else
                            currTextRunIdx += currTextRun->getLength();
                    }

                    // find the smaller of the two lengths, because that's
                    // what we're going to draw
                    int lengthToDraw;
                    if(currFragment->getLength() < currTextRun->getLength())
                        lengthToDraw = currFragment->getLength();
                    else
                        lengthToDraw = currTextRun->getLength();

                    // determine if there are any parts that are selected
                    int unselectedFragment1Length = 0,
                        selectedFragmentLength = 0,
                        unselectedFragment2Length = currFragment->getLength();
                    if(m_isMouseDown && currLine.hasTextSelection())
                    {
                        // cache some variables
                        int fragmentStartIdx = currTextIdx;
                        int fragmentEndIdx = currTextIdx + lengthToDraw - 1;
                        int textSelectionStart = currLine.getTextSelectionStart(),
                            textSelectionEnd = currLine.getTextSelectionEnd();

                        // case 1
                        if(fragmentStartIdx < textSelectionStart
                        && textSelectionStart <= fragmentEndIdx
                        && fragmentEndIdx < textSelectionEnd)
                        {
                            unselectedFragment1Length = textSelectionStart - fragmentStartIdx;
                            selectedFragmentLength = lengthToDraw - unselectedFragment1Length;
                            unselectedFragment2Length = 0;
                        }
                        // case 2
                        else if(textSelectionStart < fragmentStartIdx
                             && fragmentStartIdx <= textSelectionEnd
                             && textSelectionEnd < fragmentEndIdx)
                        {
                            selectedFragmentLength = textSelectionEnd - fragmentStartIdx + 1;
                            unselectedFragment2Length = lengthToDraw - selectedFragmentLength;
                        }
                        // case 3
                        else if(fragmentStartIdx <= textSelectionStart
                             && textSelectionEnd <= fragmentEndIdx)
                        {
                            unselectedFragment1Length = textSelectionStart - fragmentStartIdx;
                            unselectedFragment2Length = fragmentEndIdx - textSelectionEnd;
                            selectedFragmentLength = lengthToDraw - (unselectedFragment1Length + unselectedFragment2Length);
                        }
                        // case 4
                        else if(textSelectionStart <= fragmentStartIdx
                             && fragmentEndIdx <= textSelectionEnd)
                        {
                            selectedFragmentLength = lengthToDraw;
                            unselectedFragment2Length = 0;
                        }
                    }

                    QFont linkFont = painter.font();
                    linkFont.setUnderline(true);
                    QFontMetrics fm(linkFont);
                    painter.setFont(linkFont);
                    int x = currFragment->x();
                    int y = initialHeight + currFragment->y();
                    DRAW_UNSELECTED_LINK_TEXT(currTextIdx, unselectedFragment1Length, x, y);
                    currTextIdx += unselectedFragment1Length;
                    DRAW_SELECTED_LINK_TEXT(currTextIdx, selectedFragmentLength, x, y);
                    currTextIdx += selectedFragmentLength;
                    DRAW_UNSELECTED_LINK_TEXT(currTextIdx, unselectedFragment2Length, x, y);
                    currTextIdx += unselectedFragment2Length;
                }

                // move to the next fragment
                currFragment = currFragment->nextLinkFragment();
            }
        }

        currHeight = initialHeight - lineSpacing;
    }

    // draw border
    /*
    painter.setPen(palette().dark().color());
    painter.drawLine(          0,            0,     vpWidth,            0);
    painter.drawLine(          0,            0,           0,     vpHeight);
    painter.setPen(palette().light().color());
    painter.drawLine(          1, vpHeight - 1, vpWidth - 1, vpHeight - 1);
    painter.drawLine(vpWidth - 1,            1, vpWidth - 1, vpHeight - 1);
    */
}

//-----------------------------------//

// updates the last visible line by using the new scrollbar value
// and current line splits
void OutputControl::updateScrollbarValue(int actualValue)
{
    // iterate through all the lines, until currentValue
    // is up to actualValue; then find the index for the
    // current OutputLine, and then the wrapped line num
    int currentValue = verticalScrollBar()->maximum() + 1;
    for(int i = m_lines.length() - 1; i >= 0; --i)
    {
        int numLines = m_lines[i].getNumSplits() + 1;
        if(currentValue - numLines > actualValue)
        {
            currentValue -= numLines;
        }
        else
        {
            m_lastVisibleLineIdx = i;
            m_lastVisibleWrappedLine = actualValue - (currentValue - numLines);
            break;
        }
    }
}

} } // end namespaces