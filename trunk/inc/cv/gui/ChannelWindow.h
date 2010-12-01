/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QApplication>
#include <QString>
#include <QQueue>
#include "cv/ChannelUser.h"
#include "cv/gui/InputOutputWindow.h"

class QListWidget;
class QSplitter;

namespace cv { namespace gui {

// subject to change
const unsigned int MAX_NICK_PRIORITY = 1;

// represents a message that is to be displayed
// after the channel is officially joined (i.e. when
// the list of users is received from the server)
struct QueuedOutputMessage
{
    QString             message;
    OutputMessageType   messageType;
};

class ChannelWindow : public InputOutputWindow
{
    Q_OBJECT

protected:
    QListWidget *               m_pUserList;
    QSplitter *                 m_pSplitter;

    QList<ChannelUser *>        m_users;

    // this variable dictates whether we are fully
    // in the channel or not; we are not fully in
    // a channel until the 366 numeric has been received
    // (end of /NAMES list)
    bool                        m_inChannel;
    QQueue<QueuedOutputMessage> m_messageQueue;

public:
    ChannelWindow(QExplicitlySharedDataPointer<Session> pSharedSession,
                  QExplicitlySharedDataPointer<ServerConnectionPanel> pSharedServerConnPanel,
                  const QString &title = tr("Untitled"),
                  const QSize &size = QSize(500, 300));
    ~ChannelWindow();

    bool isChannelName(const QString &name) { return (name.compare(getWindowName(), Qt::CaseInsensitive) == 0); }

    // returns true if the user is in the channel,
    // returns false otherwise
    bool hasUser(const QString &user);

    // adds the user to the channel's userlist in the proper place
    //
    // user holds the nickname, and can include any number
    // of prefixes, as well as the user and host
    //
    // returns true if the user was added,
    // returns false otherwise
    bool addUser(const QString &user);

    // removes the user from the channel's userlist
    //
    // returns true if the user was removed,
    // returns false otherwise
    bool removeUser(const QString &user);

    // changes the user's nickname from oldNick to newNick, unless
    // the user is not in the channel
    void changeUserNick(const QString &oldNick, const QString &newNick);

    // adds the specified prefix to the user, and updates the user list display
    // (if necessary)
    void addPrefixToUser(const QString &user, const QChar &prefixToAdd);

    // removes the specified prefix from the user, and updates the user list
    // display (if necessary) - assuming the user has the given prefix
    void removePrefixFromUser(const QString &user, const QChar &prefixToRemove);

    // returns the number of users currently in the channel
    int getUserCount() { return m_users.size(); }

    // events
    void onNumericMessage(Event *evt);
    void onJoinMessage(Event *evt);
    void onKickMessage(Event *evt);
    void onModeMessage(Event *evt);
    void onNickMessage(Event *evt);
    void onNoticeMessage(Event *evt);
    void onPartMessage(Event *evt);
    void onPrivmsgMessage(Event *evt);
    void onTopicMessage(Event *evt);

    void processOutputEvent(OutputEvent *evt);

protected:
    // adds a message to the "in-limbo" queue, where messages are
    // stored before the channel has been officially "joined" (when
    // the userlist has been received)
    void enqueueMessage(const QString &msg, OutputMessageType msgType);

    // "joins" the channel by flushing all messages received since creating
    // the channel (but before the list of users was received)
    void joinChannel();

    // removes all the users from memory
    void leaveChannel();

    // handles the printing/sending of the PRIVMSG message
    void handleSay(const QString &text);

    // handles the printing/sending of the PRIVMSG ACTION message
    void handleAction(const QString &text);

    void handleTab();
    void closeEvent(QCloseEvent *event);

private:
    // adds a user by pointer to IrcChanUser
    bool addUser(ChannelUser *pUser);

    // removes a user by pointer to IrcChanUser
    void removeUser(ChannelUser *pUser);

    // finds the user within the channel, based on nickname
    // (regardless if there are prefixes or a user/host in it)
    //
    // returns a pointer to the IrcChanUser if found in the channel,
    // returns NULL otherwise
    ChannelUser *findUser(const QString &user);

signals:
    // signifies that the window is closing - this is *only*
    // for IrcStatusWindow to use
    void chanWindowClosing(ChannelWindow *pWin);
};

} } // end namespaces
