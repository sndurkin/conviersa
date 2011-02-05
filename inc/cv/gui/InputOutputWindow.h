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
#include "cv/gui/ServerConnectionPanel.h"

class QPushButton;

namespace cv {

class Session;

namespace gui {

class InputOutputWindow;

class InputEvent : public Event
{
    InputOutputWindow * m_pWindow;
    Session *           m_pSession;
    QString             m_input;

public:
    InputEvent(InputOutputWindow *pWindow,
               Session *pSession,
               const QString &input)
        : m_pSession(pSession),
          m_pWindow(pWindow),
          m_input(input)
    {
        m_pSession = pSession;
    }

    InputOutputWindow *getWindow() { return m_pWindow; }
    Session *getSession() { return m_pSession; }
    QString getInput() { return m_input; }
};

class InputOutputWindow : public OutputWindow
{
    Q_OBJECT

protected:
    QPlainTextEdit *    m_pInput;
    QList<QString>      m_pastCommands;

    QExplicitlySharedDataPointer<ServerConnectionPanel>
                        m_pSharedServerConnPanel;
    QPushButton *       m_pOpenButton;

public:
    InputOutputWindow(const QString &title = tr("Untitled"),
                      const QSize &size = QSize(500, 300));
    ~InputOutputWindow();

    void giveFocus();

    void onInput(Event *evt);

    void onNoticeMessage(Event *evt);

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
