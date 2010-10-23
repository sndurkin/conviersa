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
#include "cv/gui/WindowManager.h"
#include "cv/gui/ChannelWindow.h"
#include "cv/gui/StatusWindow.h"

#include <QScrollBar>
#include <QTextEdit>

namespace cv { namespace gui {

ChannelWindow::ChannelWindow(QExplicitlySharedDataPointer<Session> pSharedSession,
                             const QString &title/* = tr("Untitled")*/,
                             const QSize &size/* = QSize(500, 300)*/)
    : InputOutputWindow(title, size),
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

    m_pSharedSession->getEventManager()->hookEvent("onNumericMessage", MakeDelegate(this, &ChannelWindow::onNumericMessage));
    m_pSharedSession->getEventManager()->hookEvent("onJoinMessage", MakeDelegate(this, &ChannelWindow::onJoinMessage));
    m_pSharedSession->getEventManager()->hookEvent("onKickMessage", MakeDelegate(this, &ChannelWindow::onKickMessage));
    m_pSharedSession->getEventManager()->hookEvent("onModeMessage", MakeDelegate(this, &ChannelWindow::onModeMessage));
    m_pSharedSession->getEventManager()->hookEvent("onNickMessage", MakeDelegate(this, &ChannelWindow::onNickMessage));
    m_pSharedSession->getEventManager()->hookEvent("onPartMessage", MakeDelegate(this, &ChannelWindow::onPartMessage));
    m_pSharedSession->getEventManager()->hookEvent("onPrivmsgMessage", MakeDelegate(this, &ChannelWindow::onPrivmsgMessage));
    m_pSharedSession->getEventManager()->hookEvent("onTopicMessage", MakeDelegate(this, &ChannelWindow::onTopicMessage));
}

ChannelWindow::~ChannelWindow()
{
    // todo: rewrite
    m_pSharedSession->getEventManager()->unhookEvent("onNumericMessage", MakeDelegate(this, &ChannelWindow::onNumericMessage));
    m_pSharedSession->getEventManager()->unhookEvent("onJoinMessage", MakeDelegate(this, &ChannelWindow::onJoinMessage));
    m_pSharedSession->getEventManager()->unhookEvent("onKickMessage", MakeDelegate(this, &ChannelWindow::onKickMessage));
    m_pSharedSession->getEventManager()->unhookEvent("onModeMessage", MakeDelegate(this, &ChannelWindow::onModeMessage));
    m_pSharedSession->getEventManager()->unhookEvent("onNickMessage", MakeDelegate(this, &ChannelWindow::onNickMessage));
    m_pSharedSession->getEventManager()->unhookEvent("onPartMessage", MakeDelegate(this, &ChannelWindow::onPartMessage));
    m_pSharedSession->getEventManager()->unhookEvent("onPrivmsgMessage", MakeDelegate(this, &ChannelWindow::onPrivmsgMessage));
    m_pSharedSession->getEventManager()->unhookEvent("onTopicMessage", MakeDelegate(this, &ChannelWindow::onTopicMessage));
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
            if(isChannelName(msg.m_params[1]))
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
            if(isChannelName(msg.m_params[1]))
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

void ChannelWindow::onJoinMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    if(isChannelName(msg.m_params[0]))
    {
        QString textToPrint;
        QString nickJoined = parseMsgPrefix(msg.m_prefix, MsgPrefixName);
        if(m_pSharedSession->isMyNick(nickJoined))
        {
            joinChannel();
            textToPrint = QString("* You have rejoined %1").arg(msg.m_params[0]);
        }
        else
        {
            textToPrint = QString("* %1 (%2) has joined %3")
                          .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName))
                          .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixUserAndHost))
                          .arg(msg.m_params[0]);
            addUser(nickJoined);
        }

        printOutput(textToPrint, QColor(g_pCfgManager->getOptionValue("colors.ini", "join")));
    }
}

void ChannelWindow::onKickMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    if(isChannelName(msg.m_params[0]))
    {
        QString textToPrint;
        if(m_pSharedSession->isMyNick(msg.m_params[1]))
        {
            leaveChannel();
            textToPrint = "* You were kicked";
        }
        else
        {
            removeUser(msg.m_params[1]);
            textToPrint = QString("* %1 was kicked").arg(msg.m_params[1]);
        }

        textToPrint += QString(" by %2").arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName));

        bool hasReason = (msg.m_paramsNum > 2 && !msg.m_params[2].isEmpty());
        if(hasReason)
        {
            textToPrint += QString(" (%1)").arg(msg.m_params[2]);
        }

        printOutput(textToPrint, g_pCfgManager->getOptionValue("colors.ini", "kick"));
    }
}

void ChannelWindow::onModeMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    if(isChannelName(msg.m_params[0]))
    {
        QString textToPrint = QString("* %1 has set mode: ").arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName));

        // ignore first parameter
        for(int i = 1; i < msg.m_paramsNum; ++i)
        {
                textToPrint += msg.m_params[i];
                textToPrint += ' ';
        }

        bool sign = true;
        QString modes = msg.m_params[1];
        for(int modesIndex = 0, paramsIndex = 2; modesIndex < modes.size(); ++modesIndex)
        {
            if(modes[modesIndex] == '+')
            {
                sign = true;
            }
            else if(modes[modesIndex] == '-')
            {
                sign = false;
            }
            else
            {
                ChanModeType type = getChanModeType(m_pSharedSession->getChanModes(), modes[modesIndex]);
                switch(type)
                {
                    case ModeTypeA:
                    case ModeTypeB:
                    case ModeTypeC:
                    {
                        // if there's no params left then continue
                        if(paramsIndex >= msg.m_paramsNum)
                            break;

                        QChar prefix = m_pSharedSession->getPrefixRule(modes[modesIndex]);
                        if(prefix != '\0')
                        {
                            if(sign)
                                addPrefixToUser(msg.m_params[paramsIndex], prefix);
                            else
                                removePrefixFromUser(msg.m_params[paramsIndex], prefix);
                        }

                        ++paramsIndex;
                        break;
                    }
                    default:
                    {

                    }
                }
            }
        }

        printOutput(textToPrint, QColor(g_pCfgManager->getOptionValue("colors.ini", "mode")));
    }
}

void ChannelWindow::onNickMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();

    QString oldNick = parseMsgPrefix(msg.m_prefix, MsgPrefixName);
    if(hasUser(oldNick))
    {
        changeUserNick(oldNick, msg.m_params[0]);
        QString textToPrint = QString("* %1 is now known as %2")
                              .arg(oldNick)
                              .arg(msg.m_params[0]);
        printOutput(textToPrint, g_pCfgManager->getOptionValue("colors.ini", "nick"));
    }
}

void ChannelWindow::onPartMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    if(isChannelName(msg.m_params[0]))
    {
        QString textToPrint;

        // if the PART message is of you leaving the channel
        if(m_pSharedSession->isMyNick(parseMsgPrefix(msg.m_prefix, MsgPrefixName)))
        {
            leaveChannel();
            textToPrint = QString("* You have left %1").arg(msg.m_params[0]);

        }
        else
        {
            removeUser(parseMsgPrefix(msg.m_prefix, MsgPrefixName));
            textToPrint = QString("* %1 (%2) has left %3")
                          .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName))
                          .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixUserAndHost))
                          .arg(msg.m_params[0]);
        }

        // if there's a part message
        bool hasReason = (msg.m_paramsNum > 1 && !msg.m_params[1].isEmpty());
        if(hasReason)
        {
            textToPrint += QString(" (%1)").arg(msg.m_params[1]);
        }

        printOutput(textToPrint, g_pCfgManager->getOptionValue("colors.ini", "part"));
    }
}

void ChannelWindow::onPrivmsgMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();

    if(isChannelName(msg.m_params[0]))
    {
        QString fromNick = parseMsgPrefix(msg.m_prefix, MsgPrefixName);
        QString textToPrint;
        QColor color;

        CtcpRequestType requestType = getCtcpRequestType(msg);
        if(requestType != RequestTypeInvalid)
        {
            // ACTION is /me, so handle according to that
            if(requestType == RequestTypeAction)
            {
                QString action = msg.m_params[1];

                // action = "\1ACTION <action>\1"
                // first 8 characters and last 1 character need to be excluded
                // so we'll take the mid, starting at index 8 and going until every
                // character but the last is included
                color.setNamedColor(g_pCfgManager->getOptionValue("colors.ini", "action"));
                textToPrint = QString("* %1 %2")
                              .arg(fromNick)
                              .arg(action.mid(8, action.size()-9));
            }
        }
        else
        {
            color.setNamedColor(g_pCfgManager->getOptionValue("colors.ini", "say"));
            textToPrint = QString("<%1> %2")
                          .arg(fromNick)
                          .arg(msg.m_params[1]);
        }

        if(!hasFocus())
        {
            if(msg.m_params[1].toLower().contains(m_pSharedSession->getNick().toLower()))
            {
                QApplication::alert(this);
            }
        }

        printOutput(textToPrint, color);
    }
}

void ChannelWindow::onTopicMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    if(isChannelName(msg.m_params[0]))
    {
        QString textToPrint = QString("* %1 changes topic to: %2")
                    .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName))
                    .arg(msg.m_params[1]);
        QColor topicColor(g_pCfgManager->getOptionValue("colors.ini", "topic"));
        printOutput(textToPrint, topicColor);

        if(m_pManager)
        {
            QTreeWidgetItem *pItem = m_pManager->getItemFromWindow(this);
            QString titleWithTopic = QString("%1: %2")
                                     .arg(pItem->text(0))
                                     .arg(msg.m_params[1]);
            setTitle(stripCodes(titleWithTopic));
        }
    }
}

// handles the printing/sending of the PRIVMSG message
void ChannelWindow::handleSay(const QString &text)
{
    QString textToPrint = QString("<%1> %2")
                          .arg(m_pSharedSession->getNick())
                          .arg(text);
    printOutput(textToPrint, QColor(g_pCfgManager->getOptionValue("colors.ini", "say")));
    m_pSharedSession->sendPrivmsg(getWindowName(), text);
}

// handles the printing/sending of the PRIVMSG ACTION message
void ChannelWindow::handleAction(const QString &text)
{
    QString textToPrint = QString("* %1 %2")
                          .arg(m_pSharedSession->getNick())
                          .arg(text);
    printOutput(textToPrint, QColor(g_pCfgManager->getOptionValue("colors.ini", "action")));
    m_pSharedSession->sendAction(getWindowName(), text);
}

void ChannelWindow::handleTab()
{
    QString text = getInputText();
    int idx = m_pInput->textCursor().position();

    // find the beginning of the word that the user is
    // trying to tab-complete
    while(idx >= 0 && text[idx] != ' ') --idx;
    idx++;

    // now capture the word
    QString word = text.mid(idx);

    if(word.isEmpty())
    {
        // todo: beep?
        return;
    }

    QList<QString> possibleNickNames;

    for(int j = 0; j < m_users.size(); ++j)
    {
        if(m_users[j]->getNickname().toLower().startsWith(word.toLower()))
        {
            possibleNickNames.append(m_users[j]->getNickname());
        }
    }

    if(possibleNickNames.size() == 0)
    {
        // TODO: Beep?
        return;
    }
    else if(possibleNickNames.size() == 1)
    {
        QString s = text.mid(0, idx);

        m_pInput->clear();
        m_pInput->insertPlainText(s + possibleNickNames[0]);
    }
    else
    {
        m_pOutput->append("Possibilities:\n");
        for(int i = 0; i < possibleNickNames.size(); ++i)
        {
            m_pOutput->insertPlainText(possibleNickNames[i] + " ");
        }
        m_pOutput->insertPlainText("\n");
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
