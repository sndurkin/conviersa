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

Client::Client(const QString &title)
{
    setupEvents();
    setupConfig();

    setClientSize();
    setWindowTitle(title);

    setupMenu();

    m_pMainContainer = new WindowContainer(this);
    setCentralWidget(m_pMainContainer);

    m_pDock = new QDockWidget(tr("Window Manager"));
    m_pDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_pDock->setFeatures(QDockWidget::DockWidgetMovable);
    loadQSS();

    m_pManager = new WindowManager(m_pDock, m_pMainContainer);
    m_pDock->setWidget(m_pManager);
    m_pDock->setTitleBarWidget(NULL);
    addDockWidget(Qt::LeftDockWidgetArea, m_pDock);

    // create new irc server window on client start
    onNewIrcServerWindow();
}

//-----------------------------------//

Client::~Client()
{
    delete g_pCfgManager;
    delete g_pEvtManager;
}

//-----------------------------------//

void Client::closeEvent(QCloseEvent *)
{
    // save the size of the client
    g_pCfgManager->setOptionValue("client.width", QString::number(width()));
    g_pCfgManager->setOptionValue("client.height", QString::number(height()));
    g_pCfgManager->writeToDefaultFile();

    delete m_pManager;
    delete m_pDock;
    deleteLater();
}

//-----------------------------------//

void Client::setupEvents()
{
    g_pEvtManager = new EventManager;
    g_pEvtManager->createEvent("input");
    g_pEvtManager->createEvent("output");
    g_pEvtManager->createEvent("doubleClickedLink");
}

//-----------------------------------//

void Client::setupMenu()
{
    m_pFileMenu = menuBar()->addMenu(tr("&File"));

    QAction *pNewIrcWinAct = m_pFileMenu->addAction(tr("&New IRC Server"));
    QList<QKeySequence> newServerShortcuts;
    newServerShortcuts.append(QKeySequence("Ctrl+T"));
    pNewIrcWinAct->setShortcuts(newServerShortcuts);

    m_pFileMenu->addSeparator();

    QAction *pRefreshQSS = m_pFileMenu->addAction(tr("Refresh QSS"));
    QList<QKeySequence> refreshShortcuts;
    refreshShortcuts.append(QKeySequence("Ctrl+R"));
    pRefreshQSS->setShortcuts(refreshShortcuts);

    QObject::connect(pNewIrcWinAct, SIGNAL(triggered()), this, SLOT(onNewIrcServerWindow()));
    QObject::connect(pRefreshQSS, SIGNAL(triggered()), this, SLOT(loadQSS()));

    m_pFileMenu->addSeparator();
}

//-----------------------------------//

void Client::setupConfig()
{
    g_pCfgManager = new ConfigManager("conviersa.ini");

    QList<ConfigOption> defOptions;

    setupColorConfig(defOptions);
    setupServerConfig(defOptions);
    setupGeneralConfig(defOptions);
    setupMessagesConfig(defOptions);

    g_pCfgManager->setupDefaultConfigFile(defOptions);
}

//-----------------------------------//

void Client::setupColorConfig(QList<ConfigOption> &defOptions)
{
    // WindowManager colors
    defOptions.append(ConfigOption("wmanager.color.background", "#ffffff"));
    defOptions.append(ConfigOption("wmanager.color.foreground", "#000000"));

    // input colors
    defOptions.append(ConfigOption("input.color.background", "#ffffff"));
    defOptions.append(ConfigOption("input.color.foreground", "#000000"));

    // output colors
    defOptions.append(ConfigOption("output.color.say", "#000000"));
    defOptions.append(ConfigOption("output.color.background", "#ffffff"));

    defOptions.append(ConfigOption("output.color.custom1", "#ffffff"));   // white
    defOptions.append(ConfigOption("output.color.custom2", "#000000"));   // black
    defOptions.append(ConfigOption("output.color.custom3", "#000080"));   // navy
    defOptions.append(ConfigOption("output.color.custom4", "#008000"));   // green
    defOptions.append(ConfigOption("output.color.custom5", "#ff0000"));   // red
    defOptions.append(ConfigOption("output.color.custom6", "#800000"));   // maroon
    defOptions.append(ConfigOption("output.color.custom7", "#800080"));   // purple
    defOptions.append(ConfigOption("output.color.custom8", "#ffa500"));   // orange
    defOptions.append(ConfigOption("output.color.custom9", "#ffff00"));   // yellow
    defOptions.append(ConfigOption("output.color.custom10", "#00ff00"));  // lime
    defOptions.append(ConfigOption("output.color.custom11", "#008080"));  // teal
    defOptions.append(ConfigOption("output.color.custom12", "#00ffff"));  // cyan
    defOptions.append(ConfigOption("output.color.custom13", "#0000ff"));  // blue
    defOptions.append(ConfigOption("output.color.custom14", "#ff00ff"));  // magenta
    defOptions.append(ConfigOption("output.color.custom15", "#808080"));  // gray
    defOptions.append(ConfigOption("output.color.custom16", "#c0c0c0"));  // light gray

    defOptions.append(ConfigOption("output.color.say.self", "#000000"));
    defOptions.append(ConfigOption("output.color.highlight", "#ff0000"));
    defOptions.append(ConfigOption("output.color.action", "#0000cc"));
    defOptions.append(ConfigOption("output.color.ctcp", "#d80000"));
    defOptions.append(ConfigOption("output.color.notice", "#b80000"));
    defOptions.append(ConfigOption("output.color.nick", "#808080"));
    defOptions.append(ConfigOption("output.color.info", "#000000"));
    defOptions.append(ConfigOption("output.color.invite", "#808080"));
    defOptions.append(ConfigOption("output.color.join", "#006600"));
    defOptions.append(ConfigOption("output.color.part", "#006600"));
    defOptions.append(ConfigOption("output.color.kick", "#000000"));
    defOptions.append(ConfigOption("output.color.mode", "#808080"));
    defOptions.append(ConfigOption("output.color.quit", "#006600"));
    defOptions.append(ConfigOption("output.color.topic", "#006600"));
    defOptions.append(ConfigOption("output.color.wallops", "#b80000"));
    defOptions.append(ConfigOption("output.color.whois", "#000000"));

    defOptions.append(ConfigOption("output.color.debug", "#8b0000"));
    defOptions.append(ConfigOption("output.color.error", "#8b0000"));

    // channel userlist colors
    defOptions.append(ConfigOption("userlist.color.background", "#ffffff"));
    defOptions.append(ConfigOption("userlist.color.foreground", "#000000"));
}

//-----------------------------------//

void Client::setupServerConfig(QList<ConfigOption> &defOptions)
{
    // no default options
    defOptions.append(ConfigOption("server.name", ""));
    defOptions.append(ConfigOption("server.nick", ""));
    defOptions.append(ConfigOption("server.altNick", ""));
}

//-----------------------------------//

void Client::setupGeneralConfig(QList<ConfigOption> &defOptions)
{
    defOptions.append(ConfigOption("client.width", "1000"));
    defOptions.append(ConfigOption("client.height", "500"));

    defOptions.append(ConfigOption("timestamp",         "off"));
    defOptions.append(ConfigOption("timestamp.format",  "[hh:mm:ss]"));
}

//-----------------------------------//

void Client::setupMessagesConfig(QList<ConfigOption> &defOptions)
{
    // regular messages
    defOptions.append(ConfigOption("message.action",        "* %1 %2"));
    defOptions.append(ConfigOption("message.ctcp",          "[CTCP %1 (from %2)]"));
    defOptions.append(ConfigOption("message.connecting",    "* Connecting to %1 (%2)"));
    defOptions.append(ConfigOption("message.connectFailed", "* Failed to connect to server (%1)"));
    defOptions.append(ConfigOption("message.disconnected",  "* Disconnected"));
    defOptions.append(ConfigOption("message.invite",        "* %1 has invited you to %2"));
    defOptions.append(ConfigOption("message.join",          "* %1 (%2) has joined %3"));
    defOptions.append(ConfigOption("message.join.self",     "* You have joined %1"));
    defOptions.append(ConfigOption("message.kick",          "* %1 was kicked by %2"));
    defOptions.append(ConfigOption("message.kick.self",     "* You were kicked by %1"));
    defOptions.append(ConfigOption("message.rejoin",        "* You have rejoined %1"));
    defOptions.append(ConfigOption("message.mode",          "* %1 has set mode: %2"));
    defOptions.append(ConfigOption("message.nick",          "* %1 is now known as %2"));
    defOptions.append(ConfigOption("message.notice",        "-%1- %2"));
    defOptions.append(ConfigOption("message.part",          "* %1 (%2) has left %3"));
    defOptions.append(ConfigOption("message.part.self",     "* You have left %1"));
    defOptions.append(ConfigOption("message.pong",          "* PONG from %1: %2"));
    defOptions.append(ConfigOption("message.quit",          "* %1 (%2) has quit"));
    defOptions.append(ConfigOption("message.reason",        " (%1%2)"));
    defOptions.append(ConfigOption("message.say",           "<%1> %2"));
    defOptions.append(ConfigOption("message.topic",         "* %1 changes topic to: %2"));
    defOptions.append(ConfigOption("message.wallops",       "* WALLOPS from %1: %2"));

    // numeric messages
    defOptions.append(ConfigOption("message.301",           "%1 is away: %2"));
    defOptions.append(ConfigOption("message.317",           "%1 has been idle %2"));
    defOptions.append(ConfigOption("message.330",           "%1 %2: %3"));
    defOptions.append(ConfigOption("message.332",           "* Topic is: %1"));
    defOptions.append(ConfigOption("message.333.status",    "%1 topic set by %2 on %3 %4"));
    defOptions.append(ConfigOption("message.333.channel",   "* Topic set by %1 on %2 %3"));
}

//-----------------------------------//

void Client::setClientSize()
{
    bool ok;
    int width = GET_OPT("client.width").toInt(&ok);
    if(!ok)
    {
        width = 1000;
        g_pCfgManager->setOptionValue("client.width", QString::number(width));
    }
    int height = GET_OPT("client.height").toInt(&ok);
    if(!ok)
    {
        height = 500;
        g_pCfgManager->setOptionValue("client.height", QString::number(height));
    }
    resize(width, height);
}

//-----------------------------------//

// creates a blank IRC server window
void Client::onNewIrcServerWindow()
{
    StatusWindow *pWin = new StatusWindow();
    m_pManager->addWindow(pWin);
    pWin->showMaximized();
}

//-----------------------------------//

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

} } // end namespaces
