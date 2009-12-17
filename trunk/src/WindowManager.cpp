#include <new>
#include "WindowManager.h"
#include "AltWindowContainer.h"
#include "IWindow.h"

WindowManager::WindowManager(QWidget *pParent, WindowContainer *pMainContainer)
	: QTreeWidget(pParent),
	  m_pMainContainer(pMainContainer)
{
	// properties
	setHeaderHidden(true);
	
	// context menu stuff
	SetupContextMenu();
	
	QObject::connect(m_pMainContainer, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(OnSubWindowActivated(QMdiSubWindow *)));
	
	/*
	// add the orphaned window node, and hide it;
	// it is only shown when there is an orphaned window
	m_pOrphanTreeItem = AddTreeGroup("Orphaned Windows");
	m_pOrphanTreeItem->setHidden(true);
	m_numOrphans = 0;
	*/
}

WindowManager::~WindowManager()
{
	// destroy the windows
	while(!m_winList.empty())
	{
		IWindow *pWin = m_winList.front().m_pWindow;
		if(pWin->HasContainer())
		{
			// separate the window from the container
			pWin->m_pSubWindow->setWidget(NULL);
			delete pWin->m_pSubWindow;
			pWin->m_pContainer = NULL;
			pWin->m_pSubWindow = NULL;
		}
		pWin->close();
	}
	
	// destroy the main container
	if(m_pMainContainer)
	{
		delete m_pMainContainer;
	}
	
	// destroy the alternate containers
	while(!m_altContainers.empty())
	{
		m_altContainers.front()->close();
	}
}

QSize WindowManager::sizeHint() const
{
	return QSize(175, 200);
}

// adds a top-level item to the treeview
QTreeWidgetItem *WindowManager::AddTreeGroup(const char *title)
{
	QTreeWidgetItem *pNewItem = new (std::nothrow) QTreeWidgetItem(QStringList(tr(title)));
	if(!pNewItem)
		return NULL;
	addTopLevelItem(pNewItem);
	return pNewItem;
}

// manages a WindowContainer; it will handle
// deallocation when the WindowManager is destroyed
void WindowManager::AddContainer(WindowContainer *pCont)
{
	m_altContainers.append(pCont);
}

// removes the WindowContainer from the list
// of managed containers
void WindowManager::RemoveContainer(WindowContainer *pCont)
{
	m_altContainers.removeOne(pCont);
}

// manages an IWindow under the WindowManager; it
// will handle its deallocation upon closing
//
// pWin: 	pointer to the IWindow, which should point
//		to a valid IWindow
// pParent: pointer to the parent tree item to be added
//		under, or NULL if it is to become an orphaned
//		window
//
// returns: true if successful,
//		false otherwise
bool WindowManager::AddWindow(IWindow *pWin, QTreeWidgetItem *pParent)
{
	// rudimentary checking to make sure parameters are valid
	if(!pWin || !pParent)
		return false;
	
	pWin->m_pManager = this;
	MoveWindow(pWin, m_pMainContainer);
	
	win_info_t tempInfo;
	tempInfo.m_pWindow = pWin;
	
	// add the item to the tree
	QTreeWidgetItem *pNewItem = new (std::nothrow) QTreeWidgetItem(QStringList(pWin->windowTitle()));
	if(!pNewItem)
		return false;
	
	if(pParent)
	{
		//pWin->m_isOrphan = false;
		pParent->addChild(pNewItem);
		setCurrentItem(pNewItem);
		pParent->setExpanded(true);
	}/*
	else
	{
		// make it an orphan
		pWin->m_isOrphan = true;
		m_pOrphanTreeItem->addChild(pNewItem);
		if(m_numOrphans == 0)
		{
			m_pOrphanTreeItem->setExpanded(true);
			m_pOrphanTreeItem->setHidden(false);
		}
		++m_numOrphans;
	}*/
	
	tempInfo.m_pTreeItem = pNewItem;
	
	// add the structure to the list
	m_winList.append(tempInfo);
	
	return true;
}

// removes the respective item in the treeview, but has
// to let the window close itself due to Qt's design;
// if the treeview item has children, close events will
// be sent to their respective IWindows as well
//
// todo: optimize
void WindowManager::RemoveWindow(IWindow *pWin)
{
	if(!pWin)
		return;
	
	pWin->m_pManager = NULL;
	
	int index = GetIndexFromWindow(pWin);
	if(index < 0)
		return;
	
	// remove the treeview item
	QTreeWidgetItem *pItem = m_winList[index].m_pTreeItem;
	pItem->parent()->removeChild(pItem);
	delete pItem;
	/*
	// handle the case where it's an orphan
	if(pWin->IsOrphan())
	{
		if(--m_numOrphans == 0)
			m_pOrphanTreeItem->setHidden(true);
	}
	*/
	// remove it from the list
	m_winList.removeAt(index);
}

// moves the window to a window container, or to the desktop
// if the pointer to the container provided is NULL
void WindowManager::MoveWindow(IWindow *pWin, WindowContainer *pCont)
{
	if(pWin->HasContainer())
	{
		pWin->m_pContainer->removeSubWindow(pWin);
		delete pWin->m_pSubWindow;
		pWin->setParent(NULL);
	}
	
	if(pCont)
	{
		bool max = pCont->currentSubWindow() && pCont->currentSubWindow()->isMaximized();
		pWin->m_pSubWindow = pCont->addSubWindow(pWin);
		if(max)
		{
			pWin->m_pSubWindow->showMaximized();
		}
	}
	// we're moving it to the desktop
	else
	{
		// relocate it
		pWin->move(pWin->m_pContainer->pos() + pos());
		pWin->m_pSubWindow = NULL;
	}
	
	pWin->m_pContainer = pCont;
	pWin->show();
	pWin->setFocus();
}
/*
// moves the window's node in the treeview from wherever
// it is currently located to under the "Orphaned Windows"
// treeview node
void WindowManager::DisownWindow(IWindow *pWin)
{
	if(!pWin || pWin->IsOrphan())
		return;
	
	QTreeWidgetItem *pItem = GetItemFromWindow(pWin);
	if(pItem)
	{
		pWin->m_isOrphan = true;
		pItem->parent()->removeChild(pItem);
		m_pOrphanTreeItem->addChild(pItem);
		if(++m_numOrphans == 1)
		{
			m_pOrphanTreeItem->setExpanded(true);
			m_pOrphanTreeItem->setHidden(false);
		}
	}
}
*/
// returns a pointer to the window based on the title
// returns NULL if not found
IWindow *WindowManager::FindWindow(const QString &title)
{
	int size = m_winList.size();
	for(int i = 0; i < size; ++i)
	{
		if(title.compare(m_winList[i].m_pWindow->windowTitle(), Qt::CaseInsensitive) == 0)
		{
			return m_winList[i].m_pWindow;
		}
	}
	
	return NULL;
}

// returns the a pointer to the hierarchical parent window
//	of the given window
// returns NULL if the window is invalid or if there is no parent
//	(if it's a top-level window)
IWindow *WindowManager::GetParentWindow(IWindow *pWin)
{
	QTreeWidgetItem *pItem = GetItemFromWindow(pWin);
	if(!pItem)
		return NULL;
	
	return GetWindowFromItem(pItem->parent());
}

// returns a QList of pointers to IWindows that are 
// children to the parent provided
QList<IWindow *> WindowManager::GetChildWindows(IWindow *pParent)
{
	QList<IWindow *> list;
	
	QTreeWidgetItem *pItem = GetItemFromWindow(pParent);
	if(pItem)
	{
		int numChildren = pItem->childCount();
		for(int i = 0; i < numChildren; ++i)
		{
			QTreeWidgetItem *pChild = pItem->child(i);
			for(int j = 0; j < m_winList.size(); ++j)
			{
				// found a child
				if(m_winList[j].m_pTreeItem == pChild)
				{
					list.append(m_winList[j].m_pWindow);
				}
			}
		}
	}
	
	// if there are no children, list will be empty
	return list;
}

// retrieves the index in the QList given the pointer to the IWindow
int WindowManager::GetIndexFromWindow(IWindow *pWin)
{
	int size = m_winList.size();
	for(int i = 0; i < size; ++i)
	{
		if(m_winList[i].m_pWindow == pWin)
		{
			return i;
		}
	}

	return -1;
}

// retrieves the index in the QList give the pointer to the treeview item
int WindowManager::GetIndexFromItem(QTreeWidgetItem *pItem)
{
	int size = m_winList.size();
	for(int i = 0; i < size; ++i)
	{
		if(m_winList[i].m_pTreeItem == pItem)
		{
			return i;
		}
	}

	return -1;
}

// retrieves the pointer to the treeview item given the pointer to the IWindow
QTreeWidgetItem *WindowManager::GetItemFromWindow(IWindow *pWin)
{
	int size = m_winList.size();
	for(int i = 0; i < size; ++i)
	{
		if(m_winList[i].m_pWindow == pWin)
			return m_winList[i].m_pTreeItem;
	}
	
	return NULL;
}

// retrieves the pointer to the IWindow given the pointer to the treeview item
IWindow *WindowManager::GetWindowFromItem(QTreeWidgetItem *pItem)
{
	int size = m_winList.size();
	for(int i = 0; i < size; ++i)
	{
		if(m_winList[i].m_pTreeItem == pItem)
			return m_winList[i].m_pWindow;
	}
	
	return NULL;
}

// activates the window in the treeview
void WindowManager::OnSubWindowActivated(QMdiSubWindow *pSubWin)
{
	if(pSubWin == 0)
		return;
	
	for(int i = 0; i < m_winList.size(); ++i)
	{
		if(m_winList[i].m_pWindow->m_pSubWindow == pSubWin)
		{
			setCurrentItem(m_winList[i].m_pTreeItem);
			break;
		}
	}
}

void WindowManager::SetupContextMenu()
{
	m_pContextMenu = new QMenu;
	
	QMenu *pMoveMenu = new QMenu("Move window to");
	m_pContextMenu->addMenu(pMoveMenu);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction("hi");
	
	m_pActMoveDesktop = new QAction("Desktop", pMoveMenu);
	pMoveMenu->addAction(m_pActMoveDesktop);
	
	pMoveMenu->addSeparator();
	
	m_pActMoveMainCont = new QAction("Main Container", pMoveMenu);
	pMoveMenu->addAction(m_pActMoveMainCont);
	
	m_pActMoveNewCont = new QAction("New Container...", pMoveMenu);
	pMoveMenu->addAction(m_pActMoveNewCont);
}

//
// Overridden event functions
//
void WindowManager::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::RightButton)
	{
		event->accept();
		return;
	}
	else if(event->button() == Qt::LeftButton)
	{
		// todo: do stuff to select window
		QTreeWidgetItem *pItem = itemAt(event->pos());
		if(pItem)
		{
			int index = GetIndexFromItem(pItem);
			if(index >= 0)
			{
				IWindow *pWin = m_winList[index].m_pWindow;
				if(pWin->HasContainer())
				{
					// todo: fix for multiple containers
					pWin->m_pSubWindow->setFocus();
				}
				else
				{
					pWin->activateWindow();
				}
				
				pWin->GiveFocus();
			}
		}
		
		QTreeWidget::mousePressEvent(event);
		event->ignore();
	}
}

void WindowManager::contextMenuEvent(QContextMenuEvent *event)
{
	QTreeWidgetItem *pItem = itemAt(event->pos());
	if(pItem)
	{
		for(int i = 0; i < invisibleRootItem()->childCount(); ++i)
		{
			if(invisibleRootItem()->child(i) == pItem)
			{
				// it's a top-level item, such as "IRC"
				return;
			}
		}
		
		int index = GetIndexFromItem(pItem);
		if(index >= 0)
		{
			// show the context menu
			QAction *pAct = m_pContextMenu->exec(event->globalPos());
			if(pAct == m_pActMoveDesktop)
			{
				// move window to desktop
				MoveWindow(m_winList[index].m_pWindow, NULL);
			}
			else if(pAct == m_pActMoveMainCont)
			{
				// move it to the main container
				MoveWindow(m_winList[index].m_pWindow, m_pMainContainer);
			}
			else if(pAct == m_pActMoveNewCont)
			{
				WindowContainer *pNewCont = new AltWindowContainer(this);
				AddContainer(pNewCont);
				MoveWindow(m_winList[index].m_pWindow, pNewCont);
			}
		}
	}
}
