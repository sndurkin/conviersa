/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include "cv/ConfigManager.h"
#include "cv/gui/Client.h"
#include "cv/gui/WindowManager.h"
#include "cv/gui/AltWindowContainer.h"
#include "cv/gui/InputOutputWindow.h"
#include "cv/gui/StatusWindow.h"
#include "cv/gui/ChannelWindow.h"
#include "cv/gui/QueryWindow.h"

namespace cv {

ConfigManager * g_pCfgManager;
EventManager *  g_pEvtManager;

namespace gui {

//-----------------------------------//

Client::Client(const QString &title)
{
    setWindowTitle(title);
    resize(700, 500);

    setupMenu();

    m_pMainContainer = new WindowContainer(this);
    setCentralWidget(m_pMainContainer);

    m_pDock = new QDockWidget(tr("Window Manager"));
    m_pDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    m_pManager = new WindowManager(m_pDock, m_pMainContainer);
    m_pDock->setWidget(m_pManager);
    addDockWidget(Qt::LeftDockWidgetArea, m_pDock);

    m_irc = m_pManager->addTreeGroup("IRC");
    onNewIrcServerWindow();

    g_pCfgManager = new ConfigManager;
    setupColorConfig();

    // TODO: fill in with events
    g_pEvtManager = new EventManager;
    g_pEvtManager->createEvent("onInput");
    g_pEvtManager->hookEvent("onInput", &InputOutputWindow::handleInput);
}

//-----------------------------------//

void Client::closeEvent(QCloseEvent *event)
{
    delete m_pManager;
    delete m_pDock;
    deleteLater();
}

//-----------------------------------//

void Client::setupMenu()
{
    m_pFileMenu = menuBar()->addMenu(tr("&File"));

    QAction *pNewIrcWinAct = m_pFileMenu->addAction(tr("&New IRC Server"));
    QList<QKeySequence> list;
    QKeySequence keysq("Ctrl+T");
    list.append(keysq);
    pNewIrcWinAct->setShortcuts(list);
    QObject::connect(pNewIrcWinAct, SIGNAL(triggered(bool)), this, SLOT(onNewIrcServerWindow()));

    m_pFileMenu->addSeparator();
}

//-----------------------------------//

void Client::setupColorConfig()
{
    QList<ConfigOption> defOptions;
    defOptions.append(ConfigOption("say", "#000000"));
    defOptions.append(ConfigOption("action", "#0000CC"));
    defOptions.append(ConfigOption("topic", "#006600"));
    defOptions.append(ConfigOption("join", "#006600"));
    defOptions.append(ConfigOption("part", "#006600"));
    defOptions.append(ConfigOption("quit", "#006600"));
    defOptions.append(ConfigOption("kick", "#000000"));
    defOptions.append(ConfigOption("nick", "#808080"));
    defOptions.append(ConfigOption("invite", "#808080"));
    defOptions.append(ConfigOption("mode", "#808080"));
    defOptions.append(ConfigOption("notice", "#B80000"));
    defOptions.append(ConfigOption("ctcp", "#D80000"));
    defOptions.append(ConfigOption("other", "#D80000"));
    g_pCfgManager->setupConfigFile("colors.ini", defOptions);
}

//-----------------------------------//

// creates a blank IRC server window
void Client::onNewIrcServerWindow()
{
    StatusWindow *pWin = new StatusWindow();
    m_pManager->addWindow(pWin, m_irc);
    pWin->showMaximized();
}

//-----------------------------------//

} } // end namespaces
