#ifndef WINDOWCONTAINER_H
#define WINDOWCONTAINER_H

#include <QMdiArea>
#include <QMdiSubWindow>

class IWindow;

// custom QMdiArea that i'm using as a container
// to hold IWindows
class WindowContainer : public QMdiArea
{
protected:
	QList<IWindow *>		m_windows;
	
public:
	WindowContainer(QWidget* pParent);
	virtual ~WindowContainer() { }
	
	int Size() const { return m_windows.size(); }
};

#endif
