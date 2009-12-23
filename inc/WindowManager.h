/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QTreeWidget>
#include <QMouseEvent>
#include <QMenu>
#include <QList>
#include "definitions.h"

class QMdiSubWindow;
class WindowContainer;
class IWindow;

class WindowManager : public QTreeWidget
{
	Q_OBJECT
	
private:
	WindowContainer *			m_pMainContainer;
	QList<WindowContainer *>	m_altContainers;
	QList<win_info_t>			m_winList;
	
	QMenu	*				m_pContextMenu;
	QAction *				m_pActMoveDesktop;
	QAction *				m_pActMoveMainCont;
	QAction *				m_pActMoveNewCont;
	QList<QAction *>			m_actsMoveAltCont;
	
	/*
	QTreeWidgetItem *			m_pOrphanTreeItem;
	int					m_numOrphans;
	*/
public:
	WindowManager(QWidget *pParent, WindowContainer *pMainContainer);
	~WindowManager();
	
	// adds a top-level item to the treeview
	QTreeWidgetItem *AddTreeGroup(const char *title);
	
	// manages a WindowContainer; it will handle
	// deallocation when the WindowManager is destroyed
	void AddContainer(WindowContainer *pCont);
	
	// removes the WindowContainer from the list
	// of managed containers
	void RemoveContainer(WindowContainer *pCont);
	
	// manages an IWindow under the WindowManager; it
	// will handle its deallocation upon closing
	//
	// pWin: pointer to the IWindow, which should point
	//		to a valid IWindow
	// pParent: pointer to the parent tree item to be added
	//		under, should also be valid
	//
	// returns: true if successful,
	//		false otherwise
	bool AddWindow(IWindow *pWin, QTreeWidgetItem *pParent);
	
	// removes the respective item in the treeview, but has
	// to let the window close itself due to Qt's design
	void RemoveWindow(IWindow *pWin);
	
	// moves the window to a window container, or to the desktop
	// if the pointer to the container provided is NULL
	void MoveWindow(IWindow *pWin, WindowContainer *pCont);

	// returns a pointer to the window based on the title
	// returns NULL if not found
	IWindow *FindWindow(const QString &title);
	
	// returns the a pointer to the hierarchical parent window
	//	of the given window
	// returns NULL if the window is invalid or if there is no parent
	//	(if it's a top-level window)
	IWindow *GetParentWindow(IWindow *pWin);
	
	// returns a QList of pointers to IWindows that are 
	// children to the parent provided
	QList<IWindow *> GetChildWindows(IWindow *pParent);
	
	// retrieves the index in the QList given the pointer to the IWindow
	int GetIndexFromWindow(IWindow *pWin);
	
	// retrieves the index in the QList given the pointer to the treeview item
	int GetIndexFromItem(QTreeWidgetItem *pItem);
	
	// retrieves the pointer to the treeview item given the pointer to the IWindow
	QTreeWidgetItem *GetItemFromWindow(IWindow *pWin);
	
	// retrieves the pointer to the IWindow given the pointer to the treeview item
	IWindow *GetWindowFromItem(QTreeWidgetItem *pItem);
	
	QSize sizeHint() const;
	
public slots:
	// activates the window in the treeview
	void OnSubWindowActivated(QMdiSubWindow *pSubWin);

protected:
	void SetupContextMenu();
	
	void mousePressEvent(QMouseEvent *event);
	void contextMenuEvent(QContextMenuEvent *event);
};
