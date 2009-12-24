/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include "Client.h"
#include "WindowManager.h"
#include "ConfigManager.h"
#include "AltWindowContainer.h"
#include "IrcStatusWindow.h"
#include "IrcChanWindow.h"
#include "IrcPrivWindow.h"

ConfigManager *g_pCfgManager;

namespace cv {

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
    IrcStatusWindow *pWin = new IrcStatusWindow();
    m_pManager->addWindow(pWin, m_irc);
    pWin->showMaximized();
}

//-----------------------------------//

} // end namespace
