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

    // returns a pointer to the IIrcWindow if it exists,
    // 	and is a child of this status window (meaning
    //	it can only be a channel or PM window)
    // returns NULL otherwise
    OutputWindow *getChildIrcWindow(const QString &name);

    // returns true if the child window with the provided name
    // exists, returns false otherwise
    bool childIrcWindowExists(const QString &name);

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

    // events
    void onServerConnect(Event *evt);
    void onServerDisconnect(Event *evt);
    void onReceiveData(Event *evt);
    void onErrorMessage(Event *evt);
    void onInviteMessage(Event *evt);
    void onJoinMessage(Event *evt);
    void onModeMessage(Event *evt);
    void onNickMessage(Event *evt);
    void onNoticeMessage(Event *evt);
    void onPongMessage(Event *evt);
    void onPrivmsgMessage(Event *evt);
    void onQuitMessage(Event *evt);
    void onWallopsMessage(Event *evt);
    void onNumericMessage(Event *evt);
    void onUnknownMessage(Event *evt);

protected:
    // handles the printing/sending of the PRIVMSG message
    void handleSay(const QString &text);

    // handles the printing/sending of the PRIVMSG ACTION message
    void handleAction(const QString &text);

    void handleTab();

    // handles child widget events
    bool eventFilter(QObject *obj, QEvent *event);

private:
    // numeric messages
    void handle321Numeric(const Message &msg);
    void handle322Numeric(const Message &msg);
    void handle323Numeric(const Message &msg);
    void handle353Numeric(const Message &msg);
    void handle366Numeric(const Message &msg);

public slots:
    // removes a channel window from the list
    void removeChannelWindow(ChannelWindow *pChanWin);

    // removes a PM window from the list
    void removeQueryWindow(QueryWindow *pChanWin);
};

} } // end namespaces
