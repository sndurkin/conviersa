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
#include "cv/ChannelUser.h"
#include "cv/Session.h"
#include "cv/Connection.h"
#include "cv/ConfigManager.h"
#include "cv/gui/WindowManager.h"
#include "cv/gui/ChannelWindow.h"
#include "cv/gui/StatusWindow.h"
#include "cv/gui/OutputControl.h"

#include <QScrollBar>
#include <QTextEdit>

namespace cv { namespace gui {

ChannelWindow::ChannelWindow(QExplicitlySharedDataPointer<Session> pSharedSession,
                             QExplicitlySharedDataPointer<ServerConnectionPanel> pSharedServerConnPanel,
                             const QString &title/* = tr("Untitled")*/,
                             const QSize &size/* = QSize(500, 300)*/)
    : InputOutputWindow(title, size),
      m_inChannel(false)
{
    m_pSharedSession = pSharedSession;
    m_pSharedServerConnPanel = pSharedServerConnPanel;

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

    m_pOpenButton = m_pSharedServerConnPanel->addOpenButton(m_pOutput, "Connect", 80, 30);
    m_pOutput->installEventFilter(this);

    EventManager *pEvtMgr = m_pSharedSession->getEventManager();
    pEvtMgr->hookEvent("onNumericMessage",  MakeDelegate(this, &ChannelWindow::onNumericMessage));
    pEvtMgr->hookEvent("onJoinMessage",     MakeDelegate(this, &ChannelWindow::onJoinMessage));
    pEvtMgr->hookEvent("onKickMessage",     MakeDelegate(this, &ChannelWindow::onKickMessage));
    pEvtMgr->hookEvent("onModeMessage",     MakeDelegate(this, &ChannelWindow::onModeMessage));
    pEvtMgr->hookEvent("onNoticeMessage",   MakeDelegate(this, &ChannelWindow::onNoticeMessage));
    pEvtMgr->hookEvent("onNickMessage",     MakeDelegate(this, &ChannelWindow::onNickMessage));
    pEvtMgr->hookEvent("onPartMessage",     MakeDelegate(this, &ChannelWindow::onPartMessage));
    pEvtMgr->hookEvent("onPrivmsgMessage",  MakeDelegate(this, &ChannelWindow::onPrivmsgMessage));
    pEvtMgr->hookEvent("onTopicMessage",    MakeDelegate(this, &ChannelWindow::onTopicMessage));
}

ChannelWindow::~ChannelWindow()
{
    // todo: rewrite
    EventManager *pEvtMgr = m_pSharedSession->getEventManager();
    pEvtMgr->unhookEvent("onNumericMessage",    MakeDelegate(this, &ChannelWindow::onNumericMessage));
    pEvtMgr->unhookEvent("onJoinMessage",       MakeDelegate(this, &ChannelWindow::onJoinMessage));
    pEvtMgr->unhookEvent("onKickMessage",       MakeDelegate(this, &ChannelWindow::onKickMessage));
    pEvtMgr->unhookEvent("onModeMessage",       MakeDelegate(this, &ChannelWindow::onModeMessage));
    pEvtMgr->unhookEvent("onNickMessage",       MakeDelegate(this, &ChannelWindow::onNickMessage));
    pEvtMgr->unhookEvent("onNoticeMessage",     MakeDelegate(this, &ChannelWindow::onNoticeMessage));
    pEvtMgr->unhookEvent("onPartMessage",       MakeDelegate(this, &ChannelWindow::onPartMessage));
    pEvtMgr->unhookEvent("onPrivmsgMessage",    MakeDelegate(this, &ChannelWindow::onPrivmsgMessage));
    pEvtMgr->unhookEvent("onTopicMessage",      MakeDelegate(this, &ChannelWindow::onTopicMessage));

    m_pSharedSession.reset();
    m_pSharedServerConnPanel.reset();
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
    ChannelUser *pNewUser = new ChannelUser(m_pSharedSession, user);
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

    ChannelUser *pUser = new ChannelUser(m_pSharedSession, newNick);
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

                QString textToPrint = g_pCfgManager->getOptionValue("messages.ini", "332")
                                        .arg(msg.m_params[2]);

                if(m_inChannel)
                    printOutput(textToPrint, MESSAGE_IRC_TOPIC);
                else
                    enqueueMessage(textToPrint, MESSAGE_IRC_TOPIC);
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
                QString textToPrint = g_pCfgManager->getOptionValue("messages.ini", "333-channel")
                                      .arg(msg.m_params[2])
                                      .arg(getDate(msg.m_params[3]))
                                      .arg(getTime(msg.m_params[3]));
                if(m_inChannel)
                    printOutput(textToPrint, MESSAGE_IRC_TOPIC);
                else
                    enqueueMessage(textToPrint, MESSAGE_IRC_TOPIC);
            }

            break;
        }
        case 353:
        {
            // msg.m_params[0]: my nick
            // msg.m_params[1]: "=" | "*" | "@"
            // msg.m_params[2]: channel
            // msg.m_params[3]: names, separated by spaces
            //
            // RPL_NAMREPLY was sent as a result of a JOIN command
            if(!m_inChannel)
            {
                int numSections = msg.m_params[3].count(' ') + 1;
                for(int i = 0; i < numSections; ++i)
                    addUser(msg.m_params[3].section(' ', i, i, QString::SectionSkipEmpty));
            }
            break;
        }
        case 366:
        {
            // msg.m_params[0]: my nick
            // msg.m_params[1]: channel
            // msg.m_params[2]: "End of NAMES list"
            //
            // RPL_ENDOFNAMES was sent as a result of a JOIN command
            if(!m_inChannel)
                joinChannel();
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
            textToPrint = g_pCfgManager->getOptionValue("messages.ini", "rejoin")
                            .arg(msg.m_params[0]);
        }
        else
        {
            textToPrint = g_pCfgManager->getOptionValue("messages.ini", "join")
                          .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName))
                          .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixUserAndHost))
                          .arg(msg.m_params[0]);
            addUser(nickJoined);
        }

        printOutput(textToPrint, MESSAGE_IRC_JOIN);
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
            textToPrint = g_pCfgManager->getOptionValue("messages.ini", "kick-self")
                            .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName));
        }
        else
        {
            removeUser(msg.m_params[1]);
            textToPrint = g_pCfgManager->getOptionValue("messages.ini", "kick-self")
                            .arg(msg.m_params[1])
                            .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName));
        }

        bool hasReason = (msg.m_paramsNum > 2 && !msg.m_params[2].isEmpty());
        if(hasReason)
            textToPrint += QString(" (%1%2)").arg(msg.m_params[2]).arg(QString::fromUtf8("\xF"));

        printOutput(textToPrint, MESSAGE_IRC_KICK);
    }
}

void ChannelWindow::onModeMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    if(isChannelName(msg.m_params[0]))
    {
        // ignore first parameter
        QString modeParams = msg.m_params[1];
        for(int i = 2; i < msg.m_paramsNum; ++i)
            modeParams += ' ' + msg.m_params[i];

        QString textToPrint = g_pCfgManager->getOptionValue("messages.ini", "mode")
                                .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName))
                                .arg(modeParams);

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

        printOutput(textToPrint, MESSAGE_IRC_MODE);
    }
}

void ChannelWindow::onNickMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();

    QString oldNick = parseMsgPrefix(msg.m_prefix, MsgPrefixName);
    if(hasUser(oldNick))
    {
        changeUserNick(oldNick, msg.m_params[0]);
        QString textToPrint = g_pCfgManager->getOptionValue("messages.ini", "nick")
                              .arg(oldNick)
                              .arg(msg.m_params[0]);
        printOutput(textToPrint, MESSAGE_IRC_NICK);
    }
}

void ChannelWindow::onNoticeMessage(Event *evt)
{
    if(m_pManager->isWindowFocused(this))
    {
        InputOutputWindow::onNoticeMessage(evt);
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
            textToPrint = g_pCfgManager->getOptionValue("messages.ini", "part-self")
                            .arg(msg.m_params[0]);

        }
        else
        {
            removeUser(parseMsgPrefix(msg.m_prefix, MsgPrefixName));
            textToPrint = g_pCfgManager->getOptionValue("messages.ini", "part")
                          .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName))
                          .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixUserAndHost))
                          .arg(msg.m_params[0]);
        }

        // if there's a part message
        bool hasReason = (msg.m_paramsNum > 1 && !msg.m_params[1].isEmpty());
        if(hasReason)
            textToPrint += g_pCfgManager->getOptionValue("messages.ini", "reason")
                            .arg(msg.m_params[1])
                            .arg(QString::fromUtf8("\xF"));

        printOutput(textToPrint, MESSAGE_IRC_PART);
    }
}

void ChannelWindow::onPrivmsgMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();

    if(isChannelName(msg.m_params[0]))
    {
        QString fromNick = parseMsgPrefix(msg.m_prefix, MsgPrefixName);
        QString textToPrint;
        //QColor color;
        OutputMessageType msgType;

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
                msgType = MESSAGE_IRC_ACTION;
                textToPrint = g_pCfgManager->getOptionValue("messages.ini", "action")
                              .arg(fromNick)
                              .arg(action.mid(8, action.size()-9));
            }
        }
        else
        {
            msgType = MESSAGE_IRC_SAY;
            textToPrint = g_pCfgManager->getOptionValue("messages.ini", "say")
                          .arg(fromNick)
                          .arg(msg.m_params[1]);
        }
/*
        if(!hasFocus())
        {
            if(msg.m_params[1].toLower().contains(m_pSharedSession->getNick().toLower()))
            {
                QApplication::alert(this);
            }
        }
*/
        printOutput(textToPrint, msgType);
    }
}

void ChannelWindow::onTopicMessage(Event *evt)
{
    Message msg = dynamic_cast<MessageEvent *>(evt)->getMessage();
    if(isChannelName(msg.m_params[0]))
    {
        QString textToPrint = g_pCfgManager->getOptionValue("messages.ini", "topic")
                                .arg(parseMsgPrefix(msg.m_prefix, MsgPrefixName))
                                .arg(msg.m_params[1]);
        printOutput(textToPrint, MESSAGE_IRC_TOPIC);

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

void ChannelWindow::processOutputEvent(OutputEvent *evt)
{
    for(int i = 0; i < m_users.size(); ++i)
    {
        QRegExp regex(OutputWindow::s_invalidNickPrefix
                    + QRegExp::escape(m_users[i]->getNickname())
                    + OutputWindow::s_invalidNickSuffix);
        regex.setCaseSensitivity(Qt::CaseInsensitive);
        int lastIdx = 0, idx;
        while((idx = regex.indexIn(evt->getText(), lastIdx)) >= 0)
        {
            idx += regex.capturedTexts()[1].length();
            lastIdx = idx + m_users[i]->getNickname().length() - 1;
            evt->addLinkInfo(idx, lastIdx);
        }
    }
}

// adds a message to the "in-limbo" queue, where messages are
// stored before the channel has been officially "joined" (when
// the userlist has been received)
void ChannelWindow::enqueueMessage(const QString &msg, OutputMessageType msgType)
{
    QueuedOutputMessage qom;
    qom.message = msg;
    qom.messageType = msgType;
    m_messageQueue.enqueue(qom);
}

// "joins" the channel by flushing all messages received since creating
// the channel (but before the list of users was received)
void ChannelWindow::joinChannel()
{
    m_inChannel = true;
    while(!m_messageQueue.isEmpty())
    {
        QueuedOutputMessage &qom = m_messageQueue.dequeue();
        printOutput(qom.message, qom.messageType);
    }
}

// removes all the users from memory
void ChannelWindow::leaveChannel()
{
    m_inChannel = false;
    while(m_users.size() > 0)
    {
        delete m_users.takeAt(0);
        m_pUserList->takeItem(0);
    }
}

// handles the printing/sending of the PRIVMSG message
void ChannelWindow::handleSay(const QString &text)
{
    QString textToPrint = g_pCfgManager->getOptionValue("messages.ini", "say")
                          .arg(m_pSharedSession->getNick())
                          .arg(text);
    printOutput(textToPrint, MESSAGE_IRC_SAY_SELF);
    m_pSharedSession->sendPrivmsg(getWindowName(), text);
}

// handles the printing/sending of the PRIVMSG ACTION message
void ChannelWindow::handleAction(const QString &text)
{
    QString textToPrint = g_pCfgManager->getOptionValue("messages.ini", "action")
                          .arg(m_pSharedSession->getNick())
                          .arg(text);
    printOutput(textToPrint, MESSAGE_IRC_ACTION_SELF);
    m_pSharedSession->sendAction(getWindowName(), text);
}

void ChannelWindow::handleTab()
{
    QString text = getInputText();
    int idx = m_pInput->textCursor().position();

    // find the beginning of the word that the user is
    // trying to tab-complete
    while(idx >= 0 && text[idx] != ' ') --idx;
    ++idx;

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
}


void ChannelWindow::closeEvent(QCloseEvent *event)
{
    emit chanWindowClosing(this);

    if(m_inChannel)
    {
        leaveChannel();
        m_pSharedSession->sendData(QString("PART %1").arg(getWindowName()));
    }

    return Window::closeEvent(event);
}

// adds a user by pointer to IrcChanUser
bool ChannelWindow::addUser(ChannelUser *pNewUser)
{
    QListWidgetItem *pListItem = new QListWidgetItem(pNewUser->getProperNickname());
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
        if(QString::compare(m_users[i]->getNickname(), ircChanUser.getNickname(), Qt::CaseInsensitive) == 0)
            return m_users[i];

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
