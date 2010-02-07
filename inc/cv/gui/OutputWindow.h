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

class QTextEdit;
class QMutex;
class QTimer;

namespace cv { namespace gui {

class OutputWindowScrollBar;

//-----------------------------------//

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
    QTextEdit *             m_pOutput;
    QTimer *                m_pResizeMarginTimer;
    QFont                   m_defaultFont;

    QExplicitlySharedDataPointer<Session>
                            m_pSharedSession;

    QTextCodec *            m_pCodec;

    // custom scroll bar for searching within an IRC window;
    // lines on which items are found will be draw inside
    // the slider area (proportional to the size of the slider area)
    OutputWindowScrollBar *    m_pScrollBar;

public:
    OutputWindow(const QString &title = tr("Untitled"),
                 const QSize &size = QSize(500, 300));

    // printing functions
    void printOutput(const QString &text, const QColor &color = QColor(0, 0, 0));
    void printError(const QString &text);
    void printDebug(const QString &text);

protected:
    // imitates Google Chrome's search, with lines drawn in the scrollbar
    // and keywords highlighted in the document
    void search(const QString &textToFind);

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
