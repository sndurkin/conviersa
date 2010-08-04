/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QApplication>
#include <QTimer>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextLine>
#include <QAbstractTextDocumentLayout>
#include <QWebView>
#include <QWebFrame>
#include "cv/qext.h"
#include "cv/Parser.h"
#include "cv/Session.h"
#include "cv/ConfigManager.h"
#include "cv/gui/OutputWindow.h"
#include "cv/gui/OutputWindowScrollBar.h"

namespace cv { namespace gui {

//-----------------------------------//

OutputWindow::OutputWindow(const QString &title/* = tr("Untitled")*/,
                           const QSize &size/* = QSize(500, 300)*/)
    : Window(title, size),
      // TODO: remove hardcode
      m_defaultFont("Arial", 10)
{
    // TODO: Choose an appropriate system default font...
    setFont(m_defaultFont);

    m_pCodec = QTextCodec::codecForLocale();

    m_pVLayout = new QVBoxLayout;

    m_pResizeMarginTimer = new QTimer;
    m_pResizeMarginTimer->setSingleShot(true);
    //connect(m_pResizeMarginTimer, SIGNAL(timeout()), this, SLOT(ResizeTopMargin()));

    m_pOutput = new QTextEdit;
    m_pOutput->setTabChangesFocus(true);
    m_pOutput->setReadOnly(true);
    m_pOutput->installEventFilter(this);

    m_pScrollBar = new OutputWindowScrollBar(this);
    m_pOutput->setVerticalScrollBar(m_pScrollBar);
}

//-----------------------------------//

void OutputWindow::printOutput(const QString &text, const QColor &color/* = QColor(0, 0, 0)*/)
{
    QTextCursor cursor(m_pOutput->document());
    cursor.movePosition(QTextCursor::End);

    bool atDocumentEnd = (m_pScrollBar->maximum() - m_pScrollBar->value() <= 5);

    if(!m_pOutput->document()->isEmpty())
        cursor.insertBlock();
    QString textToPrint = escapeEx(stripCodes(text));
/*
    if(text.contains(m_pSharedSession->getNick()))
    {
        textToPrint.prepend("<b>");
        textToPrint.append("</b>");
        QApplication::beep();
    }

    QRegExp URL("lol(([\\w:]+)[/]{2})?(\\w|.)+");

    if(URL.exactMatch("http://www.google.com"))
        cursor.insertText("match/n");
    else
        cursor.insertText("nawt\n");
    if(URL.exactMatch("www.google.com"))
        cursor.insertText("match\n");
    if(URL.exactMatch("lolm"))
        cursor.insertText("match\n");
*/
    cursor.insertHtml(m_pCodec->toUnicode(textToPrint.toAscii()));

    // sets the correct position of the cursor so that color
    // can be applied to the text just inserted
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    addColorsToText(text, cursor, color);

    if(atDocumentEnd && !m_pOutput->textCursor().hasSelection())
    {
        // scroll to the end of the output
        m_pOutput->moveCursor(QTextCursor::End);
        m_pOutput->ensureCursorVisible();
    }
}

//-----------------------------------//

void OutputWindow::printError(const QString &text)
{
    QString error = "[ERROR] ";
    error += text;
    printOutput(error);
}

//-----------------------------------//

void OutputWindow::printDebug(const QString &text)
{
    QString debug = "[DEBUG] ";
    debug += text;
    printOutput(debug);
}

//-----------------------------------//

// imitates Google Chrome's search, with lines drawn in the scrollbar
// and keywords highlighted in the document
void OutputWindow::search(const QString &textToFind)
{
    // reset the search lines
    m_pScrollBar->clearLines();

    // create the list of ExtraSelection items to set
    QList<QTextEdit::ExtraSelection> list;

    QTextCursor cursor(m_pOutput->document());
    QTextBlock currBlock = m_pOutput->document()->begin();
    qreal currHeight = 0.0;
    qreal totalHeight = m_pOutput->document()->size().height();
    while(true)
    {
        cursor = m_pOutput->document()->find(textToFind, cursor);
        if(cursor.isNull())
            break;

        QTextEdit::ExtraSelection extraSel;
        extraSel.cursor = cursor;
        QTextCharFormat format;
        QColor color("red");
        QBrush brush(color);
        format.setBackground(brush);
        extraSel.format = format;
        list.append(extraSel);

        while(currBlock.isValid() && currBlock.blockNumber() != cursor.blockNumber())
        {
            // keep track of the current block's height in the document
            currHeight += m_pOutput->document()->documentLayout()->blockBoundingRect(currBlock).size().height();

            currBlock = currBlock.next();
        }

        // add in the height of the QTextLine (where the cursor
        // is located) within the QTextBlock
        int relativePos = cursor.position() - cursor.block().position();
        QTextLine line = cursor.block().layout()->lineForTextPosition(relativePos);
        if(line.isValid())
        {
            // add a search line to the scrollbar
            //
            // todo: add line.height() ?
            m_pScrollBar->addLine((currHeight + line.y()) / totalHeight);
        }
    }

    m_pOutput->setExtraSelections(list);
}

//-----------------------------------//

// handles child widget events
bool OutputWindow::eventFilter(QObject *obj, QEvent *event)
{
    /*
    if(obj == m_pOutput)
    {
        if(event->type() == QEvent::Resize)
        {
            QResizeEvent *resizeEvent = static_cast<QResizeEvent *>(event);

            QString hi = "resizeEvent: ";
            hi += QString::number(resizeEvent->size().width());
            hi += ",";
            hi += QString::number(resizeEvent->size().height());
            m_pOutput->append(hi);


            if(!m_pResizeMarginTimer->isActive())
            {
                m_pResizeMarginTimer->start(30);
            }

            resizeTopMargin();
        }
    }
    */

    return Window::eventFilter(obj, event);
}

//-----------------------------------//

/*
// changes the vertical offset that ensures that the
// text always starts at the bottom of the screen
// (for the user)
void OutputWindow::resizeTopMargin()
{
    QTextFrameFormat format;
    m_startOfText = m_pOutput->height() - 36;
    format.setTopMargin(m_startOfText);
    m_pOutput->document()->rootFrame()->setFrameFormat(format);
}
*/

} } // end namespaces
