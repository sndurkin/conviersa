/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include "cv/gui/OutputWindow.h"

namespace cv { namespace gui {

class QueryWindow : public OutputWindow
{
    Q_OBJECT

private:
    QString     m_targetNick;

public:
    QueryWindow(QExplicitlySharedDataPointer<Session> pSharedSession,
                const QString &targetNick);
    ~QueryWindow();

    int getIrcWindowType();

    // changes the nickname of the person we're chatting with
    void setTargetNick(const QString &nick);

    // returns the target nick that we're chatting with
    // (same as IWindow::GetWindowName() & IWindow::GetTitle())
    QString getTargetNick();

    bool isTargetNick(const QString &nick) { return (m_targetNick.compare(nick, Qt::CaseSensitive) == 0); }

    // events
    void onNumericMessage(Event *evt);
    void onNickMessage(Event *evt);
    void onPrivmsgMessage(Event *evt);

protected:
    void handleTab();
    void closeEvent(QCloseEvent *event);

signals:
    // signifies that the window is closing - this is *only*
    // for IrcStatusWindow to use
    void privWindowClosing(QueryWindow *pWin);
};

} } // end namespaces
