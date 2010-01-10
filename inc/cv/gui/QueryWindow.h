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
                const QString &title = tr("Untitled"),
                const QSize &size = QSize(500, 300));
    ~QueryWindow();

    int getIrcWindowType();

    // changes the nickname of the person we're chatting with
    void setTargetNick(const QString &nick);

    // returns the target nick that we're chatting with
    // (same as IWindow::GetWindowName() & IWindow::GetTitle())
    QString getTargetNick();

protected:
    void handleTab();
    void closeEvent(QCloseEvent *event);

signals:
    // signifies that the window is closing - this is *only*
    // for IrcStatusWindow to use
    void privWindowClosing(QueryWindow *pWin);

public slots:
    void onServerConnect();
    void onServerDisconnect();
    void onReceiveMessage(const Message &msg);
};

} } // end namespaces
