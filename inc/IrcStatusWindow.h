/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include "IIrcWindow.h"
#include "IrcParser.h"

namespace cv { namespace irc {

class IrcChanListWindow;
class IrcChanWindow;
class IrcPrivWindow;

class IrcStatusWindow : public IIrcWindow
{
    Q_OBJECT

protected:
    bool                    m_populatingUserList;
    bool                    m_sentListStopMsg;

    IrcChanListWindow *     m_pChanListWin;
    QList<IrcChanWindow *>  m_chanList;
    QList<IrcPrivWindow *>  m_privList;

public:
    IrcStatusWindow(const QString &title = tr("Server Window"),
            const QSize &size = QSize(500, 300));
    virtual ~IrcStatusWindow();

    int getIrcWindowType();

    // handles the data received from the Connection class
    void handleData(QString &data);

    // returns a pointer to the IIrcWindow if it exists,
    // 	and is a child of this status window (meaning
    //	it can only be a channel or PM window)
    // returns NULL otherwise
    IIrcWindow *getChildIrcWindow(const QString &name);

    // returns a list of all IIrcWindows under the IrcStatusWindow
    QList<IIrcWindow *> getChildIrcWindows();

    // returns a list of all IrcChanWindows that are currently
    // being managed in the server
    QList<IrcChanWindow *> getChannels();

    // returns a list of all IrcPrivWindows that are currently
    // being managed in the server
    QList<IrcPrivWindow *> getPrivateMessages();

    // adds a channel to the list
    void addChanWindow(IrcChanWindow *pChan);

    // adds a PM window to the list
    void addPrivWindow(IrcPrivWindow *pPriv);

protected:
    void handleTab();

    // handles child widget events
    bool eventFilter(QObject *obj, QEvent *event);

private:
    // numeric messages
    void handle001Numeric(const IrcMessage &msg);
    void handle002Numeric(const IrcMessage &msg);
    void handle005Numeric(const IrcMessage &msg);
    void handle301Numeric(const IrcMessage &msg);
    void handle317Numeric(const IrcMessage &msg);
    void handle321Numeric(const IrcMessage &msg);
    void handle322Numeric(const IrcMessage &msg);
    void handle323Numeric(const IrcMessage &msg);
    void handle330Numeric(const IrcMessage &msg);
    void handle332Numeric(const IrcMessage &msg);
    void handle333Numeric(const IrcMessage &msg);
    void handle353Numeric(const IrcMessage &msg);
    void handle366Numeric(const IrcMessage &msg);
    void handle401Numeric(const IrcMessage &msg);

    // other messages
    void handleInviteMsg(const IrcMessage &msg);
    void handleJoinMsg(const IrcMessage &msg);
    void handleKickMsg(const IrcMessage &msg);
    void handleModeMsg(const IrcMessage &msg);
    void handleNickMsg(const IrcMessage &msg);
    void handleNoticeMsg(const IrcMessage &msg);
    void handlePartMsg(const IrcMessage &msg);
    void handlePongMsg(const IrcMessage &msg);
    void handlePrivMsg(const IrcMessage &msg);
    void handleQuitMsg(const IrcMessage &msg);
    void handleTopicMsg(const IrcMessage &msg);
    void handleWallopsMsg(const IrcMessage &msg);

public slots:
    // handles a disconnection fired from the Connection object
    void handleDisconnect();

    // removes a channel window from the list
    void removeChanWindow(IrcChanWindow *pChanWin);

    // removes a PM window from the list
    void removePrivWindow(IrcPrivWindow *pChanWin);
};

} } // end namespaces
