#ifndef IWINDOW_H
#define IWINDOW_H

#include <QMdiSubWindow>

class WindowManager;
class WindowContainer;

class IWindow : public QWidget
{
	friend class WindowManager;
	friend class WindowContainer;

protected:
	// we need a pointer to the current container instance
	// to be able to destroy it when there are no more
	// tabs in a desktop window
	WindowContainer *		m_pContainer;
	
	// this is used to maintain the default size of the window
	QSize				m_defSize;
	
	// we need this pointer so every IWindow instance has the ability
	// to change other IWindow instances (through the WindowManager)
	WindowManager *		m_pManager;
	
	// we need this so we can put our IWindow inside a WindowContainer
	// (it has to be inside a QMdiSubWindow so everything will work right)
	QMdiSubWindow *		m_pSubWindow;
	/*
	// dictates whether the window's treeview node is located
	// under a parent, or its own node; we affectionately call
	// these windows "orphans"
	bool				m_isOrphan;
	*/
public:
	IWindow(const QString &title = tr("Untitled"),
			const QSize &size = QSize(500, 300));
	virtual ~IWindow() { }
	
	QSize sizeHint() const;
	
	// returns the title of the window, whether it's
	// in a container or on the desktop
	QString GetTitle() const;
	
	// sets the title of the window, whether it's
	// in a container or on the desktop
	void SetTitle(const QString &title);
	
	// the name returned is the short name displayed in the WindowManager
	QString GetWindowName();
	
	// sets the short name of the window (displayed in the WM)
	void SetWindowName(const QString &name);
	
	bool HasContainer() const { return m_pContainer != NULL; }
	/*
	bool IsOrphan() { return m_isOrphan; }
	*/
	virtual void GiveFocus() = 0;
	
protected:
	virtual void closeEvent(QCloseEvent *event);
	void resizeEvent(QResizeEvent *event);
};

#endif