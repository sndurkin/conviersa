/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QVBoxLayout>
#include <QPlainTextEdit>
#include "cv/Session.h"
#include "cv/gui/Window.h"
#include "cv/gui/OutputControl.h"

class QTextEdit;

namespace cv { namespace gui {

class OutputWindowScrollBar;
class OutputControl;

//-----------------------------------//


enum OutputMessageType {

    // IRC types
    MESSAGE_IRC_SAY,
    MESSAGE_IRC_SAY_SELF,
    MESSAGE_IRC_ACTION,
    MESSAGE_IRC_CTCP,

    MESSAGE_IRC_ERROR,
    MESSAGE_IRC_INVITE,
    MESSAGE_IRC_JOIN,
    MESSAGE_IRC_KICK,
    MESSAGE_IRC_MODE,
    MESSAGE_IRC_NICK,
    MESSAGE_IRC_NOTICE,
    MESSAGE_IRC_PART,
    MESSAGE_IRC_PING,
    MESSAGE_IRC_PONG,
    MESSAGE_IRC_QUIT,
    MESSAGE_IRC_TOPIC,
    MESSAGE_IRC_WALLOPS,

    MESSAGE_IRC_NUMERIC,

    // general client types
    MESSAGE_INFO,
    MESSAGE_ERROR,
    MESSAGE_DEBUG,

    // custom type
    MESSAGE_CUSTOM

};


/*
 *  IRC window interface
 *  + indicates an abstract class
 *
 *  + Window
 *      - ChannelListWindow
 *      + OutputWindow
 *          - DebugWindow
 *          + InputOutputWindow
 *              - StatusWindow
 *              - ChannelWindow
 *              - QueryWindow
 */
class OutputWindow : public Window
{
    Q_OBJECT

protected:
    QVBoxLayout *           m_pVLayout;

    int                     m_startOfText;
    OutputControl *         m_pOutput;
    QFont                   m_defaultFont;

    QExplicitlySharedDataPointer<Session>
                            m_pSharedSession;

    // this variable holds the most important level
    // of output alert for a window not in focus; this is
    // used to determine what color to make an OutputWindow's
    // item in the WindowManager when it receives text
    // and isn't in focus
    int                     m_outputAlertLevel;

    QTextCodec *            m_pCodec;

    // custom scroll bar for searching within an IRC window;
    // lines on which items are found will be draw inside
    // the slider area (proportional to the size of the slider area)
    OutputWindowScrollBar * m_pScrollBar;

public:
    OutputWindow(const QString &title = tr("Untitled"),
                 const QSize &size = QSize(500, 300));

    // printing functions
    void printOutput(const QString &text, OutputMessageType msgType);
    void printError(const QString &text);
    void printDebug(const QString &text);

    static void handleOutput(Event *evt);
    static void handleDoubleClickLink(Event *evt);

    void focusedInTree();
    virtual void processOutputEvent(OutputEvent *evt) = 0;

protected:
    // imitates Google Chrome's search, with lines drawn in the scrollbar
    // and keywords highlighted in the document
    //void search(const QString &textToFind);

    // handles child widget events
    bool eventFilter(QObject *obj, QEvent *event);

public slots:
    // changes the vertical offset that ensures that the
    // text always starts at the bottom of the screen
    // (for the user)
    //void resizeTopMargin();
};

//-----------------------------------//

} } // end namespaces
