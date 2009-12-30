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
#include "IChatWindow.h"
#include "IrcServerInfoService.h"

class QTextEdit;
class QMutex;
class QTimer;
class QTextTable;

namespace cv { namespace irc {

class IrcWindowScrollBar;

//-----------------------------------//

/**
 * IRC window interface. Windows can be divided into different types:
 *      - Status windows
 *      - Channel windows
 *      - Private windows
 * TODO: this needs a redisign. Looks messy to me. -- triton
 */

class IIrcWindow : public IChatWindow
{
    Q_OBJECT

protected:
    QVBoxLayout *           m_pVLayout;

    int                     m_startOfText;
    QTextEdit *             m_pOutput;
    QTimer *                m_pResizeMarginTimer;
    QPlainTextEdit *        m_pInput;
    QFont                   m_defaultFont;

    QExplicitlySharedDataPointer<IrcServerInfoService>
                            m_pSharedService;

    QTextCodec *            m_pCodec;

    QList< QString >        m_pastCommands;

    // custom scroll bar for searching within an IRC window;
    // lines on which items are found will be draw inside
    // the slider area (proportional to the size of the slider area)
    IrcWindowScrollBar *    m_pScrollBar;

public:
    IIrcWindow(const QString &title = tr("Untitled"),
               const QSize &size = QSize(500, 300));

    // misc functions
    virtual void giveFocus();

    // window type functions
    virtual int getIrcWindowType() = 0;

    // printing functions
    void printOutput(const QString &text, const QColor &color = QColor(0, 0, 0));
    void printError(const QString &text);
    void printDebug(const QString &text);

protected:
    // moves the input cursor to the end of the line
    void moveCursorEnd();

    // gets the text from the input control
    QString getInputText();

    // imitates Google Chrome's search, with lines drawn in the scrollbar
    // and keywords highlighted in the document
    void search(const QString &textToFind);

    // changes the codec for the m_pOutput control
    void changeCodec(const QString &codecStr);

    // handles child widget events
    bool eventFilter(QObject *obj, QEvent *event);

    // handles the input for the window
    void handleInput(const QString &inputText);

    virtual void handleTab() = 0;

public slots:
    // changes the vertical offset that ensures that the
    // text always starts at the bottom of the screen
    // (for the user)
    //void resizeTopMargin();
};

//-----------------------------------//

} } // end namespaces
