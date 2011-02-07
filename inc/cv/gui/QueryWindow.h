/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QApplication>
#include "cv/gui/InputOutputWindow.h"

namespace cv {

class Session;

namespace gui {

class QueryWindow : public InputOutputWindow
{
    Q_OBJECT

private:
    QString     m_targetNick;

public:
    QueryWindow(Session *pSession,
                QExplicitlySharedDataPointer<ServerConnectionPanel> pSharedServerConnPanel,
                const QString &targetNick);
    ~QueryWindow();

    void setTargetNick(const QString &nick);
    QString getTargetNick();
    bool isTargetNick(const QString &nick) { return (m_targetNick.compare(nick, Qt::CaseSensitive) == 0); }

    // events
    void onNumericMessage(Event *evt);
    void onNickMessage(Event *evt);
    void onNoticeMessage(Event *evt);
    void onPrivmsgMessage(Event *evt);

    void onOutput(Event *evt);
    void onDoubleClickLink(Event *evt);

protected:
    void handleSay(const QString &text);
    void handleAction(const QString &text);
    void handleTab();

    void closeEvent(QCloseEvent *event);

signals:
    // signifies that the window is closing - this is only
    // used by StatusWindow
    void privWindowClosing(QueryWindow *pWin);
};

} } // end namespaces
