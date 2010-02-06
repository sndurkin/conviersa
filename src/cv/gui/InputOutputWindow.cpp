/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QApplication>
#include "cv/ConfigManager.h"
#include "cv/gui/InputOutputWindow.h"
#include "cv/gui/types.h"

namespace cv { namespace gui {

InputOutputWindow::InputOutputWindow(const QString &title/* = tr("Untitled")*/,
                                     const QSize &size/* = QSize(500, 300)*/)
    : OutputWindow(title, size)
{
    m_pInput = new QPlainTextEdit;
    m_pInput->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_pInput->setMaximumSize(QWIDGETSIZE_MAX, 25);
    m_pInput->installEventFilter(this);
    setFocusProxy(m_pInput);
}

//-----------------------------------//

void InputOutputWindow::giveFocus()
{
    m_pInput->setFocus();
}

//-----------------------------------//

// handles the input for the window
void InputOutputWindow::handleInput(Event *evt)
{
    // todo: change when colors are added
    InputOutputWindow *pWindow = dynamic_cast<InputEvent *>(evt)->getWindow();
    Session *pSession = dynamic_cast<InputEvent *>(evt)->getSession();
    QString text = dynamic_cast<InputEvent *>(evt)->getInput();
    QString textToPrint, textToSend;
    QColor color;

    // todo: make this changeable
    if(text[0] == '/')
    {
        // remove the first character
        text.remove(0, 1);

        // handle any special commands
        //
        // current format: /server <host> [port]
        // todo: fix channel leaving
        if(text.startsWith("server ", Qt::CaseInsensitive))
        {
            QString host = text.section(' ', 1, 1, QString::SectionSkipEmpty);
            bool ok;
            int port = text.section(' ', 2, 2, QString::SectionSkipEmpty).toInt(&ok);
            if(!ok)
            {
                // TODO: change to use config
                port = 6667;
            }

            if(pSession->isConnected())
                pSession->disconnectFromServer();
            pSession->connectToServer(host, port);

            return;
        }
        else if(text.startsWith("search ", Qt::CaseInsensitive))
        {
            text.remove(0, 7);
            pWindow->search(text);
            return;
        }
        else
        {
            if(!pSession->isConnected())
            {
                pWindow->printError("Not connected to a server.");
                return;
            }

            if(text.startsWith("me ", Qt::CaseInsensitive))
            {
                if(pWindow->getIrcWindowType() == IRC_STATUS_WIN_TYPE)
                {
                    pWindow->printError("Can't send to server status window");
                    return;
                }

                text.remove(0, 3);
                textToPrint = "* ";
                textToPrint += pSession->getNick();
                textToPrint += " ";
                textToPrint += text;
                color.setNamedColor(g_pCfgManager->getOptionValue("colors.ini", "action"));

                textToSend = "PRIVMSG ";
                textToSend += pWindow->getWindowName();
                textToSend += " :\1ACTION ";
                textToSend += text;
                textToSend += "\1";
                goto print_and_send_text;
            }
            else if(text.startsWith("say ", Qt::CaseInsensitive))
            {
                if(pWindow->getIrcWindowType() == IRC_STATUS_WIN_TYPE)
                {
                    pWindow->printError("Can't send to server status window");
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

    if(!pSession->isConnected())
    {
        pWindow->printError("Not connected to a server.");
        return;
    }

    if(pWindow->getIrcWindowType() == IRC_STATUS_WIN_TYPE)
    {
        pWindow->printError("Can't send to server status window");
        return;
    }

    textToPrint = "<";
    textToPrint += pSession->getNick();
    textToPrint += "> ";
    textToPrint += text;
    color.setNamedColor(g_pCfgManager->getOptionValue("colors.ini", "say"));

    textToSend += "PRIVMSG ";
    textToSend += pWindow->getWindowName();
    textToSend += " :";
    textToSend += text;

print_and_send_text:
    // print it
    pWindow->printOutput(textToPrint, color);

send_text:
    // send it
    pSession->sendData(textToSend);
}

//-----------------------------------//

void InputOutputWindow::moveCursorEnd()
{
    QTextCursor cursor = m_pInput->textCursor();
    cursor.setPosition(QTextCursor::End);
    m_pInput->setTextCursor(cursor);
}

//-----------------------------------//

// handles child widget events
bool InputOutputWindow::eventFilter(QObject *obj, QEvent *event)
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

                InputEvent *inputEvt = new InputEvent(this, m_pSharedSession, text);
                g_pEvtManager->fireEvent("onInput", inputEvt);
                delete inputEvt;

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
    }

done:
    return OutputWindow::eventFilter(obj, event);
}

} } // end namespaces
