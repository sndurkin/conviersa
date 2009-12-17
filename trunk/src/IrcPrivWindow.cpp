#include <QWebView>
#include <QMutex>
#include "IrcTypes.h"
#include "IrcPrivWindow.h"
#include "IrcStatusWindow.h"
#include "IrcServerInfoService.h"
#include "WindowManager.h"

IrcPrivWindow::IrcPrivWindow(QExplicitlySharedDataPointer<IrcServerInfoService> pSharedService,
				QExplicitlySharedDataPointer<Connection> pSharedConn,
				const QString &title/* = tr("Untitled")*/,
				const QSize &size/* = QSize(500, 300)*/)
	: IIrcWindow(title, size)
{
	m_pSharedConn = pSharedConn;
	m_pSharedService = pSharedService;
	m_targetNick = title;
	
	m_pVLayout->addWidget(m_pOutput);
	m_pVLayout->addWidget(m_pInput);
	m_pVLayout->setSpacing(5);
	m_pVLayout->setContentsMargins(2, 2, 2, 2);
	setLayout(m_pVLayout);
	
	QObject::connect(m_pSharedConn.data(), SIGNAL(Disconnected()), this, SLOT(HandleDisconnect()));
}

int IrcPrivWindow::GetIrcWindowType()
{
	return IRC_PRIV_WIN_TYPE;
}

// changes the nickname of the person we're chatting with
void IrcPrivWindow::SetTargetNick(const QString &nick)
{
	m_targetNick = nick;
	SetWindowName(nick);
	SetTitle(nick);
}

// returns the target nick that we're chatting with
// (same as IWindow::GetWindowName() & IWindow::GetTitle())
QString IrcPrivWindow::GetTargetNick()
{
	return m_targetNick;
}

void IrcPrivWindow::HandleTab()
{
	QString text = GetInputText();
	int idx = m_pInput->textCursor().position();
}

void IrcPrivWindow::closeEvent(QCloseEvent *event)
{
	emit PrivWindowClosing(this);
	return IWindow::closeEvent(event);
}

void IrcPrivWindow::HandleDisconnect()
{
	PrintOutput("* Disconnected");
}
