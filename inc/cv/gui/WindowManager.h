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
class Event;

namespace cv {

struct ConfigOption;

namespace gui {

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

    void addContainer(WindowContainer *pCont);
    void removeContainer(WindowContainer *pCont);
    bool addWindow(Window *pWin,
                   QTreeWidgetItem *pParent = NULL,
                   bool giveFocus = true);
    void removeWindow(Window *pWin);
    void moveWindow(Window *pWin, WindowContainer *pCont);
    Window *findWindow(const QString &title);
    Window *getParentWindow(Window *pWin);
    QList<Window *> getChildWindows(Window *pParent);

    int getIndexFromWindow(Window *pWin);
    int getIndexFromItem(QTreeWidgetItem *pItem);
    QTreeWidgetItem *getItemFromWindow(Window *pWin);
    Window *getWindowFromItem(QTreeWidgetItem *pItem);

    // returns true if the current window is focused in the tree
    // false otherwise
    bool isWindowFocused(Window *pWin) { return (getWindowFromItem(currentItem()) == pWin); }

    QSize sizeHint() const { return QSize(175, 200); }

    static void setupColorConfig(QMap<QString, ConfigOption> &defOptions);

public slots:
    void onSubWindowActivated(QMdiSubWindow *pSubWin);
    void onItemChanged(QTreeWidgetItem *pCurrent, QTreeWidgetItem *pPrevious);

protected:
    void setupColors();
    void setupContextMenu();

    void onBackgroundColorChanged(Event *pEvent);
    void onForegroundColorChanged(Event *pEvent);

    void contextMenuEvent(QContextMenuEvent *event);
};

} } // end namespaces
