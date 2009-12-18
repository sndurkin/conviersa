#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include <QString>
#include <QMenu>
#include <QMenuBar>
#include <QDockWidget>

class QTreeWidgetItem;
class WindowManager;
class WindowContainer;

class Client : public QMainWindow
{
	Q_OBJECT
	
private:
	WindowContainer *	m_pMainContainer;
	WindowManager *		m_pManager;
	
	QMenu *				m_pFileMenu;
	QDockWidget *		m_pDock;
	
	QTreeWidgetItem *	m_irc;
	
public:
	Client(const QString &title);

protected:
	void closeEvent(QCloseEvent *event);

private:
	void SetupMenu();
	void SetupColorConfig();

public slots:
	// creates a blank IRC server window
	void OnNewIrcServerWindow();
};

#endif
