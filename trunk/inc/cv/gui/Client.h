/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QMainWindow>
#include <QString>
#include <QMenu>
#include <QMenuBar>
#include <QDockWidget>

class QTreeWidgetItem;

namespace cv { namespace gui {

class WindowManager;
class WindowContainer;

//-----------------------------------//

/**
 * Main class of the IRC client. This manages the menus, toolbars and
 * docked main widgets. It holds a Window Container and also a Window Manager (wtf?)
 *
 */

class Client : public QMainWindow
{
    Q_OBJECT

private:
    WindowContainer *   m_pMainContainer;
    WindowManager *     m_pManager;

    QMenu *             m_pFileMenu;
    QDockWidget *       m_pDock;

public:
    Client(const QString &title);

protected:
    void closeEvent(QCloseEvent *event);

private:
    void setupMenu();
    void setupColorConfig();
    void setupServerConfig();
    void setupGeneralConfig();
    void setupMessagesConfig();

public slots:
    // creates a blank IRC server window
    void onNewIrcServerWindow();

    void loadQSS();
};

//-----------------------------------//

} } // end namespaces
