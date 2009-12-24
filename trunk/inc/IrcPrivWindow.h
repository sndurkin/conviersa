/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include "IIrcWindow.h"

class IrcPrivWindow : public IIrcWindow
{
    Q_OBJECT

private:
    QString     m_targetNick;

public:
    IrcPrivWindow(QExplicitlySharedDataPointer<IrcServerInfoService> pSharedService,
                  QExplicitlySharedDataPointer<Connection> pSharedConn,
                  const QString &title = tr("Untitled"),
                  const QSize &size = QSize(500, 300));

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
    void privWindowClosing(IrcPrivWindow *pWin);

public slots:
    // handles a disconnection fired from the Connection object
    void handleDisconnect();
};
