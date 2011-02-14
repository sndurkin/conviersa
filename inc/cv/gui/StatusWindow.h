/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include "cv/Parser.h"
#include "cv/gui/InputOutputWindow.h"
#include "cv/gui/ServerConnectionPanel.h"

namespace cv { namespace gui {

class ChannelListWindow;
class ChannelWindow;
class QueryWindow;

class StatusWindow : public InputOutputWindow
{
    Q_OBJECT

protected:
    bool                    m_populatingUserList;
    bool                    m_sentListStopMsg;

    ChannelListWindow *     m_pChanListWin;
    QList<ChannelWindow *>  m_chanList;
    QList<QueryWindow *>    m_privList;

public:
    StatusWindow(const QString &title = tr("Server Window"),
                 const QSize &size = QSize(500, 300));
    ~StatusWindow();

    OutputWindow *getChildIrcWindow(const QString &name);

    // returns true if the child window with the provided name
    // exists, returns false otherwise
    bool childIrcWindowExists(const QString &name) { return (getChildIrcWindow(name) != NULL); }

    QList<ChannelWindow *> getChannels();
    QList<QueryWindow *> getPrivateMessages();
    void addChannelWindow(ChannelWindow *pChan);
    void addQueryWindow(QueryWindow *pPriv, bool giveFocus);

    // events
    void onServerConnecting(Event *pEvent);
    void onServerConnectFailed(Event *pEvent);
    void onServerConnect(Event *pEvent);
    void onServerDisconnect(Event *pEvent);
    void onErrorMessage(Event *pEvent);
    void onInviteMessage(Event *pEvent);
    void onJoinMessage(Event *pEvent);
    void onModeMessage(Event *pEvent);
    void onNickMessage(Event *pEvent);
    void onPongMessage(Event *pEvent);
    void onPrivmsgMessage(Event *pEvent);
    void onQuitMessage(Event *pEvent);
    void onWallopsMessage(Event *pEvent);
    void onNumericMessage(Event *pEvent);
    void onUnknownMessage(Event *pEvent);

    void onOutput(Event *pEvent);
    void onDoubleClickLink(Event *pEvent);

protected:
    void handleSay(const QString &text);
    void handleAction(const QString &text);
    void handleTab();

    bool eventFilter(QObject *obj, QEvent *event);

private:
    // numeric messages
    void handle321Numeric(const Message &msg);
    void handle322Numeric(const Message &msg);
    void handle323Numeric(const Message &msg);
    void handle353Numeric(const Message &msg);
    void handle366Numeric(const Message &msg);

public slots:
    void removeChannelWindow(ChannelWindow *pChanWin);
    void removeQueryWindow(QueryWindow *pChanWin);
    void connectToServer(QString server, int port, QString name, QString nick, QString altNick);
};

} } // end namespaces
