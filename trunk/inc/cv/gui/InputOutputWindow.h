/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QPlainTextEdit>
#include "cv/gui/OutputWindow.h"

namespace cv { namespace gui {

class InputOutputWindow;

class InputEvent : public Event
{
    InputOutputWindow * m_pWindow;
    QExplicitlySharedDataPointer<Session>
                        m_pSharedSession;
    QString             m_input;

public:
    InputEvent(InputOutputWindow *pWindow,
               QExplicitlySharedDataPointer<Session> pSharedSession,
               const QString &input)
        : m_pWindow(pWindow),
          m_input(input)
    {
        m_pSharedSession = pSharedSession;
    }

    InputOutputWindow *getWindow() { return m_pWindow; }
    Session *getSession() { return m_pSharedSession.data(); }
    QString getInput() { return m_input; }
};

class InputOutputWindow : public OutputWindow
{
    Q_OBJECT

protected:
    QPlainTextEdit *    m_pInput;
    QList<QString>      m_pastCommands;

public:
    InputOutputWindow(const QString &title = tr("Untitled"),
                      const QSize &size = QSize(500, 300));

    void giveFocus();

    static void handleInput(Event *evt);

protected:
    // handles the printing/sending of the PRIVMSG message
    virtual void handleSay(const QString &msg) = 0;

    // handles the printing/sending of the PRIVMSG ACTION message
    virtual void handleAction(const QString &msg) = 0;

    // moves the input cursor to the end of the line
    void moveCursorEnd();

    QString getInputText() { return m_pInput->toPlainText(); }

    // handles child widget events
    bool eventFilter(QObject *obj, QEvent *event);

    virtual void handleTab() = 0;
};

} } // end namespaces
