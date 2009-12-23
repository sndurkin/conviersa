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
	QString 	m_targetNick;
	
public:
	IrcPrivWindow(QExplicitlySharedDataPointer<IrcServerInfoService> pSharedService,
			QExplicitlySharedDataPointer<Connection> pSharedConn,
			const QString &title = tr("Untitled"),
			const QSize &size = QSize(500, 300));
	
	int GetIrcWindowType();
	
	// changes the nickname of the person we're chatting with
	void SetTargetNick(const QString &nick);
	
	// returns the target nick that we're chatting with
	// (same as IWindow::GetWindowName() & IWindow::GetTitle())
	QString GetTargetNick();

protected:
	void HandleTab();
	void closeEvent(QCloseEvent *event);

signals:
	// signifies that the window is closing - this is *only*
	// for IrcStatusWindow to use
	void PrivWindowClosing(IrcPrivWindow *pWin);

public slots:
	// handles a disconnection fired from the Connection object
	void HandleDisconnect();
};
