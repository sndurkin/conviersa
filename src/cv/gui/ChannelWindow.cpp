/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QWebView>
#include <QAction>
#include <QListWidget>
#include <QSplitter>
#include <QMutex>
#include "cv/ChannelUser.h"
#include "cv/Session.h"
#include "cv/Connection.h"
#include "cv/ConfigManager.h"
#include "cv/gui/types.h"
#include "cv/gui/WindowManager.h"
#include "cv/gui/ChannelWindow.h"
#include "cv/gui/StatusWindow.h"

#include <QScrollBar>
#include <QTextEdit>

namespace cv { namespace gui {

ChannelWindow::ChannelWindow(QExplicitlySharedDataPointer<Session> pSharedSession,
                             const QString &title/* = tr("Untitled")*/,
                             const QSize &size/* = QSize(500, 300)*/)
    : OutputWindow(title, size),
      m_inChannel(false)
{
    m_pSharedSession = pSharedSession;

    m_pSplitter = new QSplitter(this);
    m_pUserList = new QListWidget;
    m_pUserList->setFont(m_defaultFont);

    m_pSplitter->addWidget(m_pOutput);
    m_pSplitter->addWidget(m_pUserList);

    if(size.width() > 150)
    {
        QList<int> sizes;
        sizes.append(size.width() - 150);
        sizes.append(150);
        m_pSplitter->setSizes(sizes);
    }
    m_pSplitter->setStretchFactor(0, 1);

    m_pVLayout->addWidget(m_pSplitter, 1);
    m_pVLayout->addWidget(m_pInput);
    m_pVLayout->setSpacing(5);
    m_pVLayout->setContentsMargins(2, 2, 2, 2);

    setLayout(m_pVLayout);

    m_pSharedSession->getEventManager()->HookEvent("onNumericMessage", MakeDelegate(this, &ChannelWindow::onNumericMessage));
    //QObject::connect(m_pSharedSession.data(), SIGNAL(connected()), this, SLOT(onServerConnect()));
    //QObject::connect(m_pSharedSession.data(), SIGNAL(disconnected()), this, SLOT(onServerDisconnect()));
    //QObject::connect(m_pSharedSession.data(), SIGNAL(dataParsed(Message)), this, SLOT(onReceiveMessage(Message)));
}

ChannelWindow::~ChannelWindow()
{
    QObject::disconnect(m_pSharedSession.data(), 0, this, 0);
}

int ChannelWindow::getIrcWindowType()
{
    return IRC_CHAN_WIN_TYPE;
}

// lets the user know that he is back inside the channel,
// whose window was already open when it was rejoined
void ChannelWindow::joinChannel()
{
    m_inChannel = true;
}

// lets the user know that he is no longer in the channel,
// but still has the window open
void ChannelWindow::leaveChannel()
{
    m_inChannel = false;
    while(m_users.size() > 0)
    {
        delete m_users.takeAt(0);
        m_pUserList->takeItem(0);
    }
}

// returns true if the user is in the channel,
// returns false otherwise
bool ChannelWindow::hasUser(const QString &user)
{
    return (findUser(user) != NULL);
}

// adds the user to the channel's userlist in the proper place
//
// user holds the nickname, and can include any number
// of prefixes, as well as the user and host
//
// returns true if the user was added,
// returns false otherwise
bool ChannelWindow::addUser(const QString &user)
{
    ChannelUser *pNewUser = new (std::nothrow) ChannelUser(m_pSharedSession, user);
    if(!pNewUser)
    {
        printError("Allocation of a new user failed.");
        return false;
    }

    if(!addUser(pNewUser))
    {
        delete pNewUser;
        return false;
    }

    return true;
}

// removes the user from the channel's userlist
bool ChannelWindow::removeUser(const QString &user)
{
    for(int i = 0; i < m_users.size(); ++i)
    {
        if(user.compare(m_users[i]->getNickname(), Qt::CaseInsensitive) == 0)
        {
            delete m_users.takeAt(i);
            m_pUserList->takeItem(i);
            return true;
        }
    }

    return false;
}

// changes the user's nickname from oldNick to newNick, unless
// the user is not in the channel
void ChannelWindow::changeUserNick(const QString &oldNick, const QString &newNick)
{
    ChannelUser *pNewUser = findUser(oldNick);
    if(!pNewUser)
        return;

    ChannelUser *pUser = new (std::nothrow) ChannelUser(m_pSharedSession, newNick);
    if(!pUser)
        return;

    for(int i = 0; i < m_users.size(); ++i)
    {
        if(m_users[i] == pNewUser)
        {
            removeUser(pNewUser);
            pNewUser->setNickname(pUser->getNickname());
            addUser(pNewUser);
            break;
        }
    }

    delete pUser;
}

// adds the specified prefix to the user, and updates the user list display
// (if necessary)
void ChannelWindow::addPrefixToUser(const QString &user, const QChar &prefixToAdd)
{
    ChannelUser *pUser = findUser(user);
    if(pUser)
    {
        // could be faster, but whatever; i'm lazy with my own API
        removeUser(pUser);
        pUser->addPrefix(prefixToAdd);
        addUser(pUser);
    }
}

// removes the specified prefix from the user, and updates the user list
// display (if necessary) - assuming the user has the given prefix
void ChannelWindow::removePrefixFromUser(const QString &user, const QChar &prefixToRemove)
{
    ChannelUser *pUser = findUser(user);
    if(pUser)
    {
        // again, i'm lazy
        removeUser(pUser);
        pUser->removePrefix(prefixToRemove);
        addUser(pUser);
    }
}

void ChannelWindow::onNumericMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    switch(msg.m_command)
    {
        // RPL_TOPIC
        case 332:
        {
            // msg.m_params[0]: my nick
            // msg.m_params[1]: channel
            // msg.m_params[2]: topic
            if(msg.m_params[1].compare(getWindowName(), Qt::CaseInsensitive) == 0)
            {
                QString titleWithTopic = QString("%1: %2")
                                         .arg(getWindowName())
                                         .arg(stripCodes(msg.m_params[2]));
                setTitle(titleWithTopic);

                QColor topicColor(g_pCfgManager->getOptionValue("colors.ini", "topic"));
                QString textToPrint = QString("* Topic is: %1").arg(msg.m_params[2]);
                printOutput(textToPrint, topicColor);
            }

            break;
        }
        case 333:
        {
            // msg.m_params[0]: my nick
            // msg.m_params[1]: channel
            // msg.m_params[2]: nick
            // msg.m_params[3]: unix time
            if(msg.m_params[1].compare(getWindowName(), Qt::CaseInsensitive) == 0)
            {
                QString textToPrint = QString("* Topic set by %1 on %2 %3")
                                      .arg(msg.m_params[2])
                                      .arg(getDate(msg.m_params[3]))
                                      .arg(getTime(msg.m_params[3]));
                printOutput(textToPrint, QColor(g_pCfgManager->getOptionValue("colors.ini", "topic")));
            }

            break;
        }
    }
}

void ChannelWindow::handleTab()
{
    QString text = getInputText();
    int idx = m_pInput->textCursor().position();

    // find the beginning of the word that the user is
    // trying to tab-complete
    while(idx >= 0 && text[idx] != ' ') --idx;

    // now capture the word
    QString word;
    int idxSpace = text.indexOf(' ', idx);
    if(idxSpace < 0)
    {
        word = text.mid(idx);
    }
    else
    {
        word = text.mid(idx, idxSpace - idx);
    }

    // if there is no word to complete, then exit
    if(word.isEmpty())
    {
        // todo: beep?
        return;
    }

    // now search in the user list for the name
    for(int i = MAX_NICK_PRIORITY; i >= 0; --i)
    {
        for(int j = 0; j < m_users.size(); ++j)
        {
            if(m_users[j]->getPriority() == i)
            {

            }
        }
    }
}

void ChannelWindow::closeEvent(QCloseEvent *event)
{
    emit chanWindowClosing(this);

    if(m_inChannel)
    {
        leaveChannel();
        QString textToSend = "PART ";
        textToSend += getWindowName();
        m_pSharedSession->sendData(textToSend);
    }

    return Window::closeEvent(event);
}

// adds a user by pointer to IrcChanUser
bool ChannelWindow::addUser(ChannelUser *pNewUser)
{
    QListWidgetItem *pListItem = new (std::nothrow) QListWidgetItem(pNewUser->getProperNickname());
    if(!pListItem)
        return false;

    for(int i = 0; i < m_users.size(); ++i)
    {
        int compareVal = m_pSharedSession->compareNickPrefixes(m_users[i]->getPrefix(), pNewUser->getPrefix());
        if(compareVal > 0)
        {
            m_pUserList->insertItem(i, pListItem);
            m_users.insert(i, pNewUser);
            return true;
        }
        else if(compareVal == 0)
        {
            compareVal = QString::compare(m_users[i]->getNickname(), pNewUser->getNickname(), Qt::CaseInsensitive);
            if(compareVal > 0)
            {
                m_pUserList->insertItem(i, pListItem);
                m_users.insert(i, pNewUser);
                return true;
            }
            else if(compareVal == 0)
            {
                // already in the list
                return false;
            }
        }
    }

    m_pUserList->addItem(pListItem);
    m_users.append(pNewUser);
    return true;
}

// removes a user by pointer to IrcChanUser
void ChannelWindow::removeUser(ChannelUser *pUser)
{
    for(int i = 0; i < m_users.size(); ++i)
    {
        if(m_users[i] == pUser)
        {
            delete m_pUserList->takeItem(i);
            m_users.removeAt(i);
            break;
        }
    }
}

// finds the user within the channel, based on nickname
// (regardless if there are prefixes or a user/host in it)
//
// returns a pointer to the IrcChanUser if found in the channel,
// returns NULL otherwise
ChannelUser *ChannelWindow::findUser(const QString &user)
{
    ChannelUser ircChanUser(m_pSharedSession, user);
    for(int i = 0; i < m_users.size(); ++i)
    {
        if(QString::compare(m_users[i]->getNickname(), ircChanUser.getNickname(), Qt::CaseInsensitive) == 0)
        {
            return m_users[i];
        }
    }

    return NULL;
}
/*
void ChannelWindow::onServerConnect() { }

void ChannelWindow::onServerDisconnect()
{
    printOutput("* Disconnected");
    m_pUserList->setEnabled(false);
}

void ChannelWindow::onReceiveMessage(const Message &msg) { }
*/
} } // end namespaces
