/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <new>
#include <QWindowsXPStyle>
#include <QStringBuilder>
#include "cv/ConfigManager.h"
#include "cv/gui/WindowManager.h"
#include "cv/gui/AltWindowContainer.h"
#include "cv/gui/Window.h"

#define COLOR_BACKGROUND tr("wmanager.color.background")
#define COLOR_FOREGROUND tr("wmanager.color.foreground")

namespace cv { namespace gui {

class EntireRowSelectionStyle : public QWindowsXPStyle
{
public:
    int styleHint(StyleHint hint, const QStyleOption *option = 0,
                  const QWidget *widget = 0, QStyleHintReturn *returnData = 0) const
    {
        if(hint == QStyle::SH_ItemView_ShowDecorationSelected)
            return int(true);
        return QWindowsXPStyle::styleHint(hint, option, widget, returnData);
    }
};

//-----------------------------------//

WindowManager::WindowManager(QWidget *pParent, WindowContainer *pMainContainer)
    : QTreeWidget(pParent),
      m_pMainContainer(pMainContainer)
{
    // properties
    setHeaderHidden(true);
    setStyle(new EntireRowSelectionStyle());
    setFocusPolicy(Qt::NoFocus);

    setupContextMenu();
    setupColors();
    g_pEvtManager->hookEvent("configChanged", COLOR_BACKGROUND, MakeDelegate(this, &WindowManager::onBackgroundColorChanged));
    g_pEvtManager->hookEvent("configChanged", COLOR_FOREGROUND, MakeDelegate(this, &WindowManager::onForegroundColorChanged));

    QObject::connect(m_pMainContainer, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(onSubWindowActivated(QMdiSubWindow *)));
    QObject::connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(onItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
}

//-----------------------------------//

WindowManager::~WindowManager()
{
    // destroy the windows
    while(!m_winList.empty())
    {
        Window *pWin = m_winList.front().m_pWindow;
        if(pWin->hasContainer())
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
        delete m_pMainContainer;

    // destroy the alternate containers
    while(!m_altContainers.empty())
        m_altContainers.front()->close();

    // unhook the event
    g_pEvtManager->unhookEvent("configChanged", COLOR_BACKGROUND, MakeDelegate(this, &WindowManager::onBackgroundColorChanged));
    g_pEvtManager->unhookEvent("configChanged", COLOR_FOREGROUND, MakeDelegate(this, &WindowManager::onForegroundColorChanged));
}

//-----------------------------------//

void WindowManager::setupColors()
{
    // background color is done by stylesheet
    setStyleSheet(QString("background-color: %1").arg(GET_OPT(COLOR_BACKGROUND)));
}

//-----------------------------------//

void WindowManager::onBackgroundColorChanged(Event *pEvent)
{
    ConfigEvent *pCfgEvent = DCAST(ConfigEvent, pEvent);
    if(QColor::isValidColor(pCfgEvent->getValue()))
        setStyleSheet(QString("background-color: %1").arg(pCfgEvent->getValue()));
}

//-----------------------------------//

void WindowManager::onForegroundColorChanged(Event *pEvent)
{
    ConfigEvent *pCfgEvent = DCAST(ConfigEvent, pEvent);
    if(QColor::isValidColor(pCfgEvent->getValue()))
    {
        QBrush newBrush(QColor(pCfgEvent->getValue()));
        for(int i = 0; i < m_winList.size(); ++i)
            if(!m_winList[i].m_pTreeItem->isSelected())
                m_winList[i].m_pTreeItem->setForeground(0, newBrush);
    }
}

//-----------------------------------//

// manages a WindowContainer; it will handle
// deallocation when the WindowManager is destroyed
void WindowManager::addContainer(WindowContainer *pCont)
{
    m_altContainers.append(pCont);
}

//-----------------------------------//

// removes the WindowContainer from the list
// of managed containers
void WindowManager::removeContainer(WindowContainer *pCont)
{
    m_altContainers.removeOne(pCont);
}

//-----------------------------------//

// manages a Window under the WindowManager; it
// will handle its deallocation upon closing
//
// returns: true if successful,
//          false otherwise
bool WindowManager::addWindow(Window *pWin,
                              QTreeWidgetItem *pParent/* = NULL*/,
                              bool giveFocus/* = true*/)
{
    // rudimentary checking to make sure window is valid
    if(!pWin)
        return false;

    pWin->m_pManager = this;
    moveWindow(pWin, m_pMainContainer);

    win_info_t tempInfo;
    tempInfo.m_pWindow = pWin;

    // add the item to the tree
    QTreeWidgetItem *pNewItem = new QTreeWidgetItem(QStringList(pWin->windowTitle()));

    if(pParent == NULL)
    {
        addTopLevelItem(pNewItem);
    }
    else
    {
        pParent->addChild(pNewItem);
        pParent->setExpanded(true);
    }

    if(giveFocus)
    {
        setCurrentItem(pNewItem);
        pWin->giveFocus();
    }

    tempInfo.m_pTreeItem = pNewItem;

    // add the structure to the list
    m_winList.append(tempInfo);

    return true;
}

//-----------------------------------//

// removes the respective item in the treeview, but has
// to let the window close itself due to Qt's design;
// if the treeview item has children, close events will
// be sent to their respective Windows as well
//
// todo: optimize
void WindowManager::removeWindow(Window *pWin)
{
    if(!pWin)
        return;

    pWin->m_pManager = NULL;

    int index = getIndexFromWindow(pWin);
    if(index < 0)
    {
        qDebug("[WM::removeWindow] Window that was provided was not found");
        return;
    }

    // remove the treeview item
    QTreeWidgetItem *pItem = m_winList[index].m_pTreeItem;
    if(pItem->parent() != NULL)
        pItem->parent()->removeChild(pItem);
    else
        removeItemWidget(pItem, 0);
    delete pItem;

    // remove it from the list
    m_winList.removeAt(index);
}

//-----------------------------------//

// moves the window to a window container, or to the desktop
// if the pointer to the container provided is NULL
void WindowManager::moveWindow(Window *pWin, WindowContainer *pCont)
{
    if(pWin->hasContainer())
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
            pWin->m_pSubWindow->showMaximized();
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

//-----------------------------------//

// returns a pointer to the window based on the title
// returns NULL if not found
Window *WindowManager::findWindow(const QString &title)
{
    int size = m_winList.size();
    for(int i = 0; i < size; ++i)
        if(title.compare(m_winList[i].m_pWindow->windowTitle(), Qt::CaseInsensitive) == 0)
            return m_winList[i].m_pWindow;

    return NULL;
}

//-----------------------------------//

// returns the a pointer to the hierarchical parent window
//	of the given window
// returns NULL if the window is invalid or if there is no parent
//	(if it's a top-level window)
Window *WindowManager::getParentWindow(Window *pWin)
{
    QTreeWidgetItem *pItem = getItemFromWindow(pWin);
    if(!pItem)
    {
        qDebug("[WM::getParentWindow] Window that was provided was not found");
        return NULL;
    }

    return getWindowFromItem(pItem->parent());
}

//-----------------------------------//

// returns a QList of pointers to Windows that are
// children to the parent provided
QList<Window *> WindowManager::getChildWindows(Window *pParent)
{
    QList<Window *> list;

    QTreeWidgetItem *pItem = getItemFromWindow(pParent);
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
    else
        qDebug("[WM::getChildWindows] Window that was provided was not found");

    // if there are no children, list will be empty
    return list;
}

//-----------------------------------//

// retrieves the index in the QList given the pointer to the Window
int WindowManager::getIndexFromWindow(Window *pWin)
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

//-----------------------------------//

// retrieves the index in the QList give the pointer to the treeview item
int WindowManager::getIndexFromItem(QTreeWidgetItem *pItem)
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

//-----------------------------------//

// retrieves the pointer to the treeview item given the pointer to the Window
QTreeWidgetItem *WindowManager::getItemFromWindow(Window *pWin)
{
    int size = m_winList.size();
    for(int i = 0; i < size; ++i)
    {
        if(m_winList[i].m_pWindow == pWin)
            return m_winList[i].m_pTreeItem;
    }

    return NULL;
}

//-----------------------------------//

// retrieves the pointer to the Window given the pointer to the treeview item
Window *WindowManager::getWindowFromItem(QTreeWidgetItem *pItem)
{
    int size = m_winList.size();
    for(int i = 0; i < size; ++i)
    {
        if(m_winList[i].m_pTreeItem == pItem)
            return m_winList[i].m_pWindow;
    }

    return NULL;
}

//-----------------------------------//

// activates the window in the treeview
void WindowManager::onSubWindowActivated(QMdiSubWindow *pSubWin)
{
    if(pSubWin == 0)
        return;

    for(int i = 0; i < m_winList.size(); ++i)
    {
        if(m_winList[i].m_pWindow->m_pSubWindow == pSubWin)
        {
            setCurrentItem(m_winList[i].m_pTreeItem);
            m_winList[i].m_pWindow->focusedInTree();
            break;
        }
    }
}

//-----------------------------------//

void WindowManager::onItemChanged(QTreeWidgetItem *pCurrent, QTreeWidgetItem *pPrevious)
{
    int index = getIndexFromItem(pCurrent);
    if(index >= 0)
    {
        Window *pWin = m_winList[index].m_pWindow;
        if(pWin->hasContainer())
        {
            // todo: fix for multiple containers
            pWin->m_pSubWindow->setFocus();
        }
        else
        {
            pWin->activateWindow();
        }

        pWin->giveFocus();
        pWin->focusedInTree();
    }

    if(pPrevious)
    {
        QFont font = pPrevious->font(0);
        font.setBold(false);
        pPrevious->setFont(0, font);
        pPrevious->setForeground(0, QBrush(QColor(GET_OPT(COLOR_FOREGROUND))));
    }

    if(pCurrent)
    {
        QFont font = pCurrent->font(0);
        font.setBold(true);
        pCurrent->setFont(0, font);
    }
}

//-----------------------------------//

void WindowManager::setupContextMenu()
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

//-----------------------------------//

//
// Overridden event functions
//
void WindowManager::contextMenuEvent(QContextMenuEvent *event)
{
    QTreeWidgetItem *pItem = itemAt(event->pos());
    if(pItem)
    {
        int index = getIndexFromItem(pItem);
        if(index >= 0)
        {
            // show the context menu
            QAction *pAct = m_pContextMenu->exec(event->globalPos());
            if(pAct == m_pActMoveDesktop)
            {
                // move window to desktop
                moveWindow(m_winList[index].m_pWindow, NULL);
            }
            else if(pAct == m_pActMoveMainCont)
            {
                // move it to the main container
                moveWindow(m_winList[index].m_pWindow, m_pMainContainer);
            }
            else if(pAct == m_pActMoveNewCont)
            {
                WindowContainer *pNewCont = new AltWindowContainer(this);
                addContainer(pNewCont);
                moveWindow(m_winList[index].m_pWindow, pNewCont);
            }
        }
    }
}

} } // end namespaces
