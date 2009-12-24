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
#include "qext.h"
#include "IrcTypes.h"
#include "IIrcWindow.h"
#include "IrcParser.h"
#include "IrcWindowScrollBar.h"
#include "IrcServerInfoService.h"
#include "ConfigManager.h"

//-----------------------------------//

IIrcWindow::IIrcWindow(const QString &title/* = tr("Untitled")*/,
                       const QSize &size/* = QSize(500, 300)*/)
    : IChatWindow(title, size),
      m_defaultFont("Arial", 10)
{
    // TODO: Choose an appropriate system default font...
    setFont(m_defaultFont);

    m_pCodec = QTextCodec::codecForLocale();

    m_pVLayout = new QVBoxLayout;

    m_pInput = new QPlainTextEdit;
    m_pInput->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_pInput->setMaximumSize(QWIDGETSIZE_MAX, 25);
    m_pInput->installEventFilter(this);
    setFocusProxy(m_pInput);

    m_pResizeMarginTimer = new QTimer;
    m_pResizeMarginTimer->setSingleShot(true);
    //connect(m_pResizeMarginTimer, SIGNAL(timeout()), this, SLOT(ResizeTopMargin()));

    m_pOutput = new QTextEdit;
    m_pOutput->setTabChangesFocus(true);
    m_pOutput->setReadOnly(true);
    m_pOutput->installEventFilter(this);

    m_pScrollBar = new IrcWindowScrollBar(this);
    m_pOutput->setVerticalScrollBar(m_pScrollBar);
}

//-----------------------------------//

void IIrcWindow::giveFocus()
{
    m_pInput->setFocus();
}

//-----------------------------------//

void IIrcWindow::printOutput(const QString &text, const QColor &color/* = QColor(0, 0, 0)*/)
{
    QTextCursor cursor(m_pOutput->document());
    cursor.movePosition(QTextCursor::End);

    bool atDocumentEnd = (m_pScrollBar->maximum() - m_pScrollBar->value() <= 5);

    if(!m_pOutput->document()->isEmpty())
        cursor.insertBlock();
    QString textToPrint = escapeEx(IrcParser::stripCodes(text));

    if(text.contains(m_pSharedService->getNick()))
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

    cursor.insertHtml(m_pCodec->toUnicode(textToPrint.toAscii()));

    // sets the correct position of the cursor so that color
    // can be applied to the text just inserted
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    IrcParser::addColorsToText(text, cursor, color);

    if(atDocumentEnd && !m_pOutput->textCursor().hasSelection())
    {
        // scroll to the end of the output
        m_pOutput->moveCursor(QTextCursor::End);
        m_pOutput->ensureCursorVisible();
    }
}

//-----------------------------------//

void IIrcWindow::printError(const QString &text)
{
    QString error = "[ERROR] ";
    error += text;
    printOutput(error);
}

//-----------------------------------//

void IIrcWindow::printDebug(const QString &text)
{
    QString debug = "[DEBUG] ";
    debug += text;
    printOutput(debug);
}

//-----------------------------------//

// gets the text from the input control
QString IIrcWindow::getInputText()
{
    return m_pInput->toPlainText();
}

//-----------------------------------//

// imitates Google Chrome's search, with lines drawn in the scrollbar
// and keywords highlighted in the document
void IIrcWindow::search(const QString &textToFind)
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

// changes the codec for the m_pOutput control
void IIrcWindow::changeCodec(const QString &codecStr)
{
    QTextCodec *pCodec = QTextCodec::codecForName(codecStr.toAscii());
    if(pCodec)
    {
        m_pCodec = pCodec;
        m_pSharedConn->setCodec(pCodec);
        QString msg = "Encoding successfully changed to \"";
        msg += codecStr;
        msg += "\"";
        printOutput(msg);
    }
}

//-----------------------------------//

void IIrcWindow::moveCursorEnd()
{
    QTextCursor cursor = m_pInput->textCursor();
    cursor.setPosition(QTextCursor::End);
    m_pInput->setTextCursor(cursor);
}

//-----------------------------------//

// handles child widget events
bool IIrcWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == m_pInput)
    {
        if(event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if(keyEvent->key() == Qt::Key_Tab)
            {
                // get the text from m_pInput and pass it off
                // to each specific IRC Window to handle
                handleTab();
                return true;
            }
            else if(keyEvent->key() == Qt::Key_Return
                    || keyEvent->key() == Qt::Key_Enter)
            {
                QString text = getInputText();
                m_pastCommands.append(text);
                m_pInput->clear();
                handleInput(text);
                return true;
            }
            else if(keyEvent->key() == Qt::Key_Up)
            {
                if(m_pastCommands.empty())
                {
                    QApplication::beep();
                    goto done;
                }

                m_pastCommands.prepend(getInputText());
                m_pInput->setPlainText(m_pastCommands.takeLast());

                moveCursorEnd();
            }
            else if(keyEvent->key() == Qt::Key_Down)
            {
                if(m_pastCommands.empty())
                {
                    QApplication::beep();
                    goto done;
                }

                m_pastCommands.append(getInputText());
                m_pInput->setPlainText(m_pastCommands.takeFirst());

                moveCursorEnd();
            }
        }

        return false;
    }/*
    else if(obj == m_pOutput)
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
        }*/

done:

    return IChatWindow::eventFilter(obj, event);
}

//-----------------------------------//

// handles the input for the window
//
// todo: decide if target is really needed
void IIrcWindow::handleInput(const QString &inputText)
{
    // todo: change when colors are added
    QString text = inputText;
    QString textToPrint, textToSend;
    QColor color;

    // todo: make this changeable
    if(text[0] == '/')
    {
        // remove the first character
        text.remove(0, 1);

        // handle any special commands
        //
        // current format: /server host port
        // todo: fix channel leaving
        if(text.startsWith("server ", Qt::CaseInsensitive))
        {
            /*
            if(!m_pSharedConn->IsConnected() && IsOrphan())
            {
                PrintError("Cannot reconnect to an IRC network through an orphaned window.");
                return;
            }
            */
            QString host = text.section(' ', 1, 1, QString::SectionSkipEmpty);
            bool ok;
            int port = text.section(' ', 2, 2, QString::SectionSkipEmpty).toInt(&ok);
            if(!ok)
            {
                printError("Invalid port.");
            }
            else
            {
                // close the connection
                if(m_pSharedConn->isConnected())
                    m_pSharedConn->disconnect();

                // detach the service from the server
                if(m_pSharedService->isAttached())
                    m_pSharedService->detachFromServer();

                if(!m_pSharedConn->connect(host.toAscii().data(), port))
                {
                    printOutput("Connect() returned false.");
                    return;
                }

                m_pSharedService->attachToServer(host, port);
            }

            return;
        }
        else if(text.startsWith("search ", Qt::CaseInsensitive))
        {
            text.remove(0, 7);
            search(text);
            return;
        }
        else if(text.startsWith("codec ", Qt::CaseInsensitive))
        {
            text.remove(0, 6);
            changeCodec(text);
            return;
        }
        else if(text.startsWith("codecs", Qt::CaseInsensitive))
        {
            QList<QByteArray> list = QTextCodec::availableCodecs();
            for(int i = 0; i < list.size(); ++i)
            {
                printOutput(list[i]);
            }
            return;
        }
        else
        {
            if(!m_pSharedConn->isConnected())
            {
                printError("Not connected to a server.");
                return;
            }

            if(text.startsWith("me ", Qt::CaseInsensitive))
            {
                if(getIrcWindowType() == IRC_STATUS_WIN_TYPE)
                {
                    printError("Can't send to server status window");
                    return;
                }

                text.remove(0, 3);
                textToPrint = "* ";
                textToPrint += m_pSharedService->getNick();
                textToPrint += " ";
                textToPrint += text;
                color.setNamedColor(g_pCfgManager->getOptionValue("colors.ini", "action"));

                textToSend = "PRIVMSG ";
                textToSend += getWindowName();
                textToSend += " :\1ACTION ";
                textToSend += text;
                textToSend += "\1";
                goto print_and_send_text;
            }
            else if(text.startsWith("say ", Qt::CaseInsensitive))
            {
                if(getIrcWindowType() == IRC_STATUS_WIN_TYPE)
                {
                    printError("Can't send to server status window");
                    return;
                }

                // falls through to get sent and printed
                text.remove(0, 4);
                color.setNamedColor(g_pCfgManager->getOptionValue("colors.ini", "say"));
            }
            else
            {
                textToSend = text;
                goto send_text;
            }
        }
    }

    if(!m_pSharedConn->isConnected())
    {
        printError("Not connected to a server.");
        return;
    }

    if(getIrcWindowType() == IRC_STATUS_WIN_TYPE)
    {
        printError("Can't send to server status window");
        return;
    }

    textToPrint = "<";
    textToPrint += m_pSharedService->getNick();
    textToPrint += "> ";
    textToPrint += text;
    color.setNamedColor(g_pCfgManager->getOptionValue("colors.ini", "say"));

    textToSend += "PRIVMSG ";
    textToSend += getWindowName();
    textToSend += " :";
    textToSend += text;

print_and_send_text:
    // print it
    printOutput(textToPrint, color);

send_text:
    // send it
    m_pSharedConn->send(textToSend);
}

//-----------------------------------//

/*
// changes the vertical offset that ensures that the
// text always starts at the bottom of the screen
// (for the user)
void IIrcWindow::resizeTopMargin()
{
    QTextFrameFormat format;
    m_startOfText = m_pOutput->height() - 36;
    format.setTopMargin(m_startOfText);
    m_pOutput->document()->rootFrame()->setFrameFormat(format);
}
*/
