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

namespace cv {

class ConfigOption;

namespace gui {

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
    ~Client();

protected:
    void closeEvent(QCloseEvent *event);

private:
    void setupMenu();
    void setupConfig();

    void setupColorConfig(QList<ConfigOption> &defOptions);
    void setupServerConfig(QList<ConfigOption> &defOptions);
    void setupGeneralConfig(QList<ConfigOption> &defOptions);
    void setupMessagesConfig(QList<ConfigOption> &defOptions);

    void setClientSize();

public slots:
    // creates a blank IRC server window
    void onNewIrcServerWindow();

    void loadQSS();
};

//-----------------------------------//

} } // end namespaces
