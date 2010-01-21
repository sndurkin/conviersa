/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QString>
#include "cv/ChannelUser.h"
#include "cv/gui/OutputWindow.h"

class QListWidget;
class QSplitter;
class QMutex;

namespace cv { namespace gui {

// subject to change
const unsigned int MAX_NICK_PRIORITY = 1;

class ChannelWindow : public OutputWindow
{
    Q_OBJECT

protected:
    QListWidget *           m_pUserList;
    QSplitter *             m_pSplitter;

    QList<ChannelUser *>    m_users;

    bool                    m_inChannel;

public:
    ChannelWindow(QExplicitlySharedDataPointer<Session> pSharedSession,
                  const QString &title = tr("Untitled"),
                  const QSize &size = QSize(500, 300));
    ~ChannelWindow();

    bool isInChannel() { return m_inChannel; }

    int getIrcWindowType();

    // lets the user know that he is back inside the channel,
    // whose window was already open when it was rejoined
    void joinChannel();

    // lets the user know that he is no longer in the channel,
    // but still has the window open
    void leaveChannel();

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

protected:
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
