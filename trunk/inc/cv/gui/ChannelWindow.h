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
class QListWidgetItem;
class QSplitter;

namespace cv {

class Session;

namespace gui {

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

    // these variables are used to keep track of
    // the set of nicks that are currently matches
    // for the autocomplete string
    QList<ChannelUser *>        m_autocompleteMatches;
    int                         m_matchesIdx;
    QString                     m_preAutocompleteStr;
    QString                     m_postAutocompleteStr;

    // this variable dictates whether we are fully
    // in the channel or not; we are not fully in
    // a channel until the 366 numeric has been received
    // (end of /NAMES list)
    bool                        m_inChannel;
    QQueue<QueuedOutputMessage> m_messageQueue;

public:
    ChannelWindow(Session *pSession,
                  QExplicitlySharedDataPointer<ServerConnectionPanel> pSharedServerConnPanel,
                  const QString &title = tr("Untitled"),
                  const QSize &size = QSize(500, 300));
    ~ChannelWindow();

    bool isChannelName(const QString &name) { return (name.compare(getWindowName(), Qt::CaseInsensitive) == 0); }

    // returns true if the user is in the channel,
    // returns false otherwise
    bool hasUser(const QString &user) { return (findUser(user) != NULL); }

    bool addUser(const QString &user);
    bool removeUser(const QString &user);
    void changeUserNick(const QString &oldNick, const QString &newNick);
    void addPrefixToUser(const QString &user, const QChar &prefixToAdd);
    void removePrefixFromUser(const QString &user, const QChar &prefixToRemove);

    // returns the number of users currently in the channel
    int getUserCount() { return m_users.size(); }

    // events
    void onNumericMessage(Event *pEvent);
    void onJoinMessage(Event *pEvent);
    void onKickMessage(Event *pEvent);
    void onModeMessage(Event *pEvent);
    void onNickMessage(Event *pEvent);
    void onNoticeMessage(Event *pEvent);
    void onPartMessage(Event *pEvent);
    void onPrivmsgMessage(Event *pEvent);
    void onTopicMessage(Event *pEvent);

    void onOutput(Event *pEvent);
    void onDoubleClickLink(Event *pEvent);

    void onColorConfigChanged(Event *pEvent);

protected:
    void setupColors();

    void enqueueMessage(const QString &msg, OutputMessageType msgType);
    void joinChannel();
    void leaveChannel();

    void handleSay(const QString &text);
    void handleAction(const QString &text);
    void handleTab();

    bool eventFilter(QObject *obj, QEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    bool addUser(ChannelUser *pUser);
    void removeUser(ChannelUser *pUser);
    ChannelUser *findUser(const QString &user);

signals:
    // signifies that the window is closing - this is only
    // used by StatusWindow
    void chanWindowClosing(ChannelWindow *pWin);

public slots:
    void onUserDoubleClicked(QListWidgetItem *pItem);
};

} } // end namespaces
