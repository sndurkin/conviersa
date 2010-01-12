/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include "cv/Parser.h"
#include "cv/gui/OutputWindow.h"

namespace cv { namespace gui {

class ChannelListWindow;
class ChannelWindow;
class QueryWindow;

class StatusWindow : public OutputWindow
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

    int getIrcWindowType();

    // returns a pointer to the IIrcWindow if it exists,
    // 	and is a child of this status window (meaning
    //	it can only be a channel or PM window)
    // returns NULL otherwise
    OutputWindow *getChildIrcWindow(const QString &name);

    // returns a list of all IIrcWindows under the IrcStatusWindow
    QList<OutputWindow *> getChildIrcWindows();

    // returns a list of all IrcChanWindows that are currently
    // being managed in the server
    QList<ChannelWindow *> getChannels();

    // returns a list of all IrcPrivWindows that are currently
    // being managed in the server
    QList<QueryWindow *> getPrivateMessages();

    // adds a channel to the list
    void addChannelWindow(ChannelWindow *pChan);

    // adds a PM window to the list
    void addQueryWindow(QueryWindow *pPriv);

protected:
    void handleTab();

    // handles child widget events
    bool eventFilter(QObject *obj, QEvent *event);

private:
    // numeric messages
    void handle001Numeric(const Message &msg);
    void handle002Numeric(const Message &msg);
    void handle005Numeric(const Message &msg);
    void handle301Numeric(const Message &msg);
    void handle317Numeric(const Message &msg);
    void handle321Numeric(const Message &msg);
    void handle322Numeric(const Message &msg);
    void handle323Numeric(const Message &msg);
    void handle330Numeric(const Message &msg);
    void handle332Numeric(const Message &msg);
    void handle333Numeric(const Message &msg);
    void handle353Numeric(const Message &msg);
    void handle366Numeric(const Message &msg);
    void handle401Numeric(const Message &msg);

    // other messages
    void handleInviteMsg(const Message &msg);
    void handleJoinMsg(const Message &msg);
    void handleKickMsg(const Message &msg);
    void handleModeMsg(const Message &msg);
    void handleNickMsg(const Message &msg);
    void handleNoticeMsg(const Message &msg);
    void handlePartMsg(const Message &msg);
    void handlePongMsg(const Message &msg);
    void handlePrivMsg(const Message &msg);
    void handleQuitMsg(const Message &msg);
    void handleTopicMsg(const Message &msg);
    void handleWallopsMsg(const Message &msg);

public slots:
    void onServerConnect();
    void onServerDisconnect();
    void onReceiveData(const QString &data);
    void onReceiveMessage(const Message &msg);

    // message-related slots
    void onErrorMessage(const Message &msg);
    void onInviteMessage(const Message &msg);
    void onJoinMessage(const Message &msg);
    void onKickMessage(const Message &msg);
    void onModeMessage(const Message &msg);
    void onNickMessage(const Message &msg);
    void onNoticeMessage(const Message &msg);
    void onPartMessage(const Message &msg);
    void onPongMessage(const Message &msg);
    void onPrivmsgMessage(const Message &msg);
    void onQuitMessage(const Message &msg);
    void onTopicMessage(const Message &msg);
    void onWallopsMessage(const Message &msg);
    void onNumericMessage(const Message &msg);

    // removes a channel window from the list
    void removeChannelWindow(ChannelWindow *pChanWin);

    // removes a PM window from the list
    void removeQueryWindow(QueryWindow *pChanWin);
};

} } // end namespaces
