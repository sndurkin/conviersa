/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QFile>
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
    g_pCfgManager = new ConfigManager;
    setupGeneralConfig();

    setWindowTitle(title);

    setupMenu();

    m_pMainContainer = new WindowContainer(this);
    setCentralWidget(m_pMainContainer);

    m_pDock = new QDockWidget(tr("Window Manager"));
    m_pDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    loadQSS();

    m_pManager = new WindowManager(m_pDock, m_pMainContainer);
    m_pDock->setWidget(m_pManager);
    addDockWidget(Qt::LeftDockWidgetArea, m_pDock);

    setupColorConfig();
    setupServerConfig();
    setupMessagesConfig();

    // create new irc server window on client start
    onNewIrcServerWindow();

    // TODO: fill in with events
    g_pEvtManager = new EventManager;
    g_pEvtManager->createEvent("onInput");
    g_pEvtManager->hookEvent("onInput", &InputOutputWindow::handleInput);
    g_pEvtManager->createEvent("onOutput");
    g_pEvtManager->hookEvent("onOutput", &OutputWindow::handleOutput);
    g_pEvtManager->createEvent("onDoubleClickLink");
    g_pEvtManager->hookEvent("onDoubleClickLink", &OutputWindow::handleDoubleClickLink);
}

//-----------------------------------//

void Client::closeEvent(QCloseEvent *event)
{
    // save the size of the client
    g_pCfgManager->setOptionValue("general.ini", "width", QString::number(width()));
    g_pCfgManager->setOptionValue("general.ini", "height", QString::number(height()));
    g_pCfgManager->writeToFile("general.ini");

    delete m_pManager;
    delete m_pDock;
    deleteLater();
}

//-----------------------------------//

void Client::setupMenu()
{
    m_pFileMenu = menuBar()->addMenu(tr("&File"));

    QAction *pNewIrcWinAct = m_pFileMenu->addAction(tr("&New IRC Server"));
    m_pFileMenu->addSeparator();
    QAction *pRefreshQSS = m_pFileMenu->addAction(tr("Refresh QSS"));
    QList<QKeySequence> list;
    QKeySequence keysq("Ctrl+T");
    list.append(keysq);
    pNewIrcWinAct->setShortcuts(list);
    QObject::connect(pNewIrcWinAct, SIGNAL(triggered()), this, SLOT(onNewIrcServerWindow()));
    QObject::connect(pRefreshQSS, SIGNAL(triggered()), this, SLOT(loadQSS()));

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

void Client::setupServerConfig()
{
    // no default options
    QList<ConfigOption> defOptions;
    defOptions.append(ConfigOption("name", ""));
    defOptions.append(ConfigOption("nick", ""));
    defOptions.append(ConfigOption("altNick", ""));
    g_pCfgManager->setupConfigFile("server.ini", defOptions);
}

//-----------------------------------//

void Client::setupGeneralConfig()
{
    QList<ConfigOption> defOptions;
    defOptions.append(ConfigOption("width", "1000"));
    defOptions.append(ConfigOption("height", "500"));
    g_pCfgManager->setupConfigFile("general.ini", defOptions);

    // set size of client
    bool ok;
    int width = g_pCfgManager->getOptionValue("general.ini", "width").toInt(&ok);
    if(!ok)
    {
        width = 1000;
        g_pCfgManager->setOptionValue("general.ini", "width", QString::number(width));
    }
    int height = g_pCfgManager->getOptionValue("general.ini", "height").toInt(&ok);
    if(!ok)
    {
        height = 500;
        g_pCfgManager->setOptionValue("general.ini", "height", QString::number(height));
    }
    resize(width, height);
}

//-----------------------------------//

void Client::setupMessagesConfig()
{
    QList<ConfigOption> defOptions;

    // regular messages
    defOptions.append(ConfigOption("action",        "* %1 %2"));
    defOptions.append(ConfigOption("ctcp",          "[CTCP %1 (from %2)]"));
    defOptions.append(ConfigOption("connecting",    "* Connecting to %1 (%2)"));
    defOptions.append(ConfigOption("connect-failed","* Failed to connect to server (%1)"));
    defOptions.append(ConfigOption("disconnected",  "* Disconnected"));
    defOptions.append(ConfigOption("invite",        "* %1 has invited you to %2"));
    defOptions.append(ConfigOption("join",          "* %1 (%2) has joined %3"));
    defOptions.append(ConfigOption("join-self",     "* You have joined %1"));
    defOptions.append(ConfigOption("kick",          "* %1 was kicked by %2"));
    defOptions.append(ConfigOption("kick-self",     "* You were kicked by %1"));
    defOptions.append(ConfigOption("rejoin",        "* You have rejoined %1"));
    defOptions.append(ConfigOption("mode",          "* %1 has set mode: %2"));
    defOptions.append(ConfigOption("nick",          "* %1 is now known as %2"));
    defOptions.append(ConfigOption("notice",        "-%1- %2"));
    defOptions.append(ConfigOption("part",          "* %1 (%2) has left %3"));
    defOptions.append(ConfigOption("part-self",     "* You have left %1"));
    defOptions.append(ConfigOption("pong",          "* PONG from %1: %2"));
    defOptions.append(ConfigOption("quit",          "* %1 (%2) has quit"));
    defOptions.append(ConfigOption("reason",        " (%1%2)"));
    defOptions.append(ConfigOption("say",           "<%1> %2"));
    defOptions.append(ConfigOption("topic",         "* %1 changes topic to: %2"));
    defOptions.append(ConfigOption("wallops",       "* WALLOPS from %1: %2"));

    // numeric messages
    defOptions.append(ConfigOption("301",           "%1 is away: %2"));
    defOptions.append(ConfigOption("317",           "%1 has been idle %2"));
    defOptions.append(ConfigOption("330",           "%1 %2: %3"));
    defOptions.append(ConfigOption("332",           "* Topic is: %1"));
    defOptions.append(ConfigOption("333-status",    "%1 topic set by %2 on %3 %4"));
    defOptions.append(ConfigOption("333-channel",   "* Topic set by %1 on %2 %3"));

    g_pCfgManager->setupConfigFile("messages.ini", defOptions);
}

//-----------------------------------//

// creates a blank IRC server window
void Client::onNewIrcServerWindow()
{
    StatusWindow *pWin = new StatusWindow();
    m_pManager->addWindow(pWin);
    pWin->showMaximized();
}

void Client::loadQSS()
{
    QFile qss("conviersa.qss");
    qss.open(QIODevice::ReadOnly);
    if(qss.isOpen())
    {
        qApp->setStyleSheet(qss.readAll());
        qss.close();
    }
}

//-----------------------------------//

} } // end namespaces
