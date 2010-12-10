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
#include "cv/gui/definitions.h"

class QMdiSubWindow;

namespace cv { namespace gui {

class WindowContainer;
class Window;

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

    // manages a Window under the WindowManager; it
    // will handle its deallocation upon closing
    //
    // returns: true if successful,
    //          false otherwise
    bool addWindow(Window *pWin,
                   QTreeWidgetItem *pParent = NULL,
                   bool giveFocus = true);

    // removes the respective item in the treeview, but has
    // to let the window close itself due to Qt's design
    void removeWindow(Window *pWin);

    // moves the window to a window container, or to the desktop
    // if the pointer to the container provided is NULL
    void moveWindow(Window *pWin, WindowContainer *pCont);

    // returns a pointer to the window based on the title
    // returns NULL if not found
    Window *findWindow(const QString &title);

    // returns the a pointer to the hierarchical parent window
    //	of the given window
    // returns NULL if the window is invalid or if there is no parent
    //	(if it's a top-level window)
    Window *getParentWindow(Window *pWin);

    // returns a QList of pointers to IWindows that are
    // children to the parent provided
    QList<Window *> getChildWindows(Window *pParent);

    // retrieves the index in the QList given the pointer to the IWindow
    int getIndexFromWindow(Window *pWin);

    // retrieves the index in the QList given the pointer to the treeview item
    int getIndexFromItem(QTreeWidgetItem *pItem);

    // retrieves the pointer to the treeview item given the pointer to the IWindow
    QTreeWidgetItem *getItemFromWindow(Window *pWin);

    // retrieves the pointer to the IWindow given the pointer to the treeview item
    Window *getWindowFromItem(QTreeWidgetItem *pItem);

    // returns true if the current window is focused in the tree
    // false otherwise
    bool isWindowFocused(Window *pWin);

    QSize sizeHint() const;

public slots:
    // activates the window in the treeview
    void onSubWindowActivated(QMdiSubWindow *pSubWin);

protected:
    void setupContextMenu();

    void mousePressEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
};

} } // end namespaces
