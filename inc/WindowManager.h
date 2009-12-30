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

namespace cv {

class WindowContainer;
class IWindow;

class WindowManager : public QTreeWidget
{
    Q_OBJECT

private:
    WindowContainer *           m_pMainContainer;
    QList<WindowContainer *>    m_altContainers;
    QList<win_info_t>           m_winList;

    QMenu   *           m_pContextMenu;
    QAction *           m_pActMoveDesktop;
    QAction *           m_pActMoveMainCont;
    QAction *           m_pActMoveNewCont;
    QList<QAction *>    m_actsMoveAltCont;

public:
    WindowManager(QWidget *pParent, WindowContainer *pMainContainer);
    ~WindowManager();

    // adds a top-level item to the treeview
    QTreeWidgetItem *addTreeGroup(const char *title);

    // manages a WindowContainer; it will handle
    // deallocation when the WindowManager is destroyed
    void addContainer(WindowContainer *pCont);

    // removes the WindowContainer from the list
    // of managed containers
    void removeContainer(WindowContainer *pCont);

    // manages an IWindow under the WindowManager; it
    // will handle its deallocation upon closing
    //
    // pWin: pointer to the IWindow, which should point
    //       to a valid IWindow
    // pParent: pointer to the parent tree item to be added
    //          under, should also be valid
    //
    // returns: true if successful,
    //          false otherwise
    bool addWindow(IWindow *pWin, QTreeWidgetItem *pParent);

    // removes the respective item in the treeview, but has
    // to let the window close itself due to Qt's design
    void removeWindow(IWindow *pWin);

    // moves the window to a window container, or to the desktop
    // if the pointer to the container provided is NULL
    void moveWindow(IWindow *pWin, WindowContainer *pCont);

    // returns a pointer to the window based on the title
    // returns NULL if not found
    IWindow *findWindow(const QString &title);

    // returns the a pointer to the hierarchical parent window
    //	of the given window
    // returns NULL if the window is invalid or if there is no parent
    //	(if it's a top-level window)
    IWindow *getParentWindow(IWindow *pWin);

    // returns a QList of pointers to IWindows that are
    // children to the parent provided
    QList<IWindow *> getChildWindows(IWindow *pParent);

    // retrieves the index in the QList given the pointer to the IWindow
    int getIndexFromWindow(IWindow *pWin);

    // retrieves the index in the QList given the pointer to the treeview item
    int getIndexFromItem(QTreeWidgetItem *pItem);

    // retrieves the pointer to the treeview item given the pointer to the IWindow
    QTreeWidgetItem *getItemFromWindow(IWindow *pWin);

    // retrieves the pointer to the IWindow given the pointer to the treeview item
    IWindow *getWindowFromItem(QTreeWidgetItem *pItem);

    QSize sizeHint() const;

public slots:
    // activates the window in the treeview
    void onSubWindowActivated(QMdiSubWindow *pSubWin);

protected:
    void setupContextMenu();

    void mousePressEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
};

} // end namespace
