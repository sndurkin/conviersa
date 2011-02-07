/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include "cv/gui/WindowContainer.h"

class QMainWindow;
class QDockWidget;
class QListWidget;
class QCloseEvent;
class QMenuBar;

namespace cv { namespace gui {

class WindowManager;

// AltWindowContainer is implemented a little differently than a
// WindowContainer, although it is derived from it so we can use
// one type in the WindowManager and Window's functions
//
// it has a parent QWidget (which sits on the desktop at all times)
// and it uses an event filter to maintain its parent
class AltWindowContainer : public WindowContainer
{
    QMainWindow *   m_pParent;

    QDockWidget *   m_pDock;
    QListWidget *   m_pList;
    QMenuBar *      m_pMenuBar;

    WindowManager * m_pManager;

public:
    AltWindowContainer(WindowManager *pManager);

protected:
    void closeEvent(QCloseEvent *event);
};

} } // end namespaces
