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
class QTimer;

namespace cv {

struct ConfigOption;

namespace gui {

class WindowManager;
class WindowContainer;

/**
 * Main class of the IRC client. This manages the menus, toolbars and
 * docked main widgets. It holds a Window Container and also a Window Manager
 */
class Client : public QMainWindow
{
    Q_OBJECT

private:
    WindowContainer *   m_pMainContainer;
    WindowManager *     m_pManager;

    QMenu *             m_pFileMenu;
    QDockWidget *       m_pDock;

    // this timer is used to find the last resize event
    // in a set of resize events that occur in quick succession
    // (like when dragging the frame to resize the window),
    // so that we can find the updated client size
    QTimer *            m_pResizeTimer;

public:
    Client(const QString &title);
    ~Client();

protected:
    void closeEvent(QCloseEvent *);
    void resizeEvent(QResizeEvent *pEvent);

private:
    void setupEvents();
    void setupMenu();
    void setupConfig();

    void setupColorConfig(QMap<QString, ConfigOption> &defOptions);
    void setupServerConfig(QMap<QString, ConfigOption> &defOptions);
    void setupGeneralConfig(QMap<QString, ConfigOption> &defOptions);
    void setupMessagesConfig(QMap<QString, ConfigOption> &defOptions);

    void setClientSize();

public slots:
    void onNewIrcServerWindow();
    void updateSizeConfig();
    void loadQSS();
};

} } // end namespaces
