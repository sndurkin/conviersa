/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

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
