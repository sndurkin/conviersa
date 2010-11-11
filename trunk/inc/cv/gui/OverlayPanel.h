/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QWidget>
#include <QPropertyAnimation>

class QPushButton;
class QSignalMapper;

namespace cv { namespace gui {

enum OverlayState
{
    OPEN,
    CLOSED
};

class OverlayPanel : public QWidget
{
    Q_OBJECT

private:
    // maintains whether the OverlayPanel has been properly
    // initialized and therefore ready for use
    bool                    m_isInitialized;

    // holds the current state of the panel (open or closed)
    OverlayState            m_state;

    // holds the alignment of the panel (and its open button, if applicable)
    Qt::AlignmentFlag       m_alignment;

    // holds the secondary alignment of the OverlayPanel; the
    // panel can be aligned to one or both of the sides, with
    // offset values available (how far to set the edges of the
    // panel, in pixels)
    //
    // if both alignments are specified, then any width for the
    // panel is ignored
    Qt::Alignment           m_secondaryAlignment;
    int                     m_firstOffset,
                            m_secondOffset;

    // the duration of the animations, in msec
    int                     m_duration;

    // the objects that perform the animations for the panel
    QPropertyAnimation *    m_pSlideOpenAnimation,
                       *    m_pSlideClosedAnimation;

    // these properties are used for an optional set of buttons
    // for opening the panel; the panel can be split across
    // multiple widgets (to conserve memory)
    //
    // the panel is shared and can only be inside one widget at a time,
    // but an open buttons is inside each widget; m_pcurrOpenButton
    // points to the button that's currently sharing the same parent
    // as the OverlayPanel
    //
    // the alignment indicates which side of the OverlayPanel
    // to align to, and the offset indicates (in pixels) how
    // far inside that edge the button is positioned
    QPushButton *           m_pCurrOpenButton;
    Qt::AlignmentFlag       m_buttonAlignment;
    int                     m_buttonOffset;
    QPropertyAnimation *    m_pOpenBtnAnimation;

    QSignalMapper *         m_pSignalMapper;

public:
    OverlayPanel(QWidget *parent);

    // realigns the OverlayPanel to the parent widget
    void realignPanel(QPushButton *pOpenButton = NULL);

    OverlayState getCurrentState() { return m_state; }

    // adds an open button for the OverlayPanel
    QPushButton *addOpenButton(QWidget *pParent, const QString &btnText, int w, int h);

    // open/close functions for panel
    void open(bool animate = true);
    void close(bool animate = true);
    void toggle(bool animate = true);

signals:
    void panelOpened();
    void panelClosed();

public slots:
    void onOpenClicked(QWidget *pButton);
    void onPanelClosed();

protected:
    // sets the duration for the OverlayPanel
    void setDuration(int duration);

    // sets the button alignment and offset
    void setButtonConfig(Qt::AlignmentFlag btnAlignment, int btnOffset);

    // sets the alignment for the OverlayPanel
    void setAlignment(Qt::AlignmentFlag alignment);

    // sets the secondary alignment for the OverlayPanel
    void setSecondaryAlignment(Qt::Alignment alignment, int firstOffset, int secondOffset);

    // sets the initial state of the OverlayPanel
    void setInitialState(OverlayState state);

    // initializes everything so that the OverlayPanel can be used; assumes
    // that the panel's current x and y are for the OPEN state
    void initialize();

    // find the offsets for the button, and move it
    void realignButton(QPushButton *pButton, int x, int y);

    // retrieve button positions for when it's open and closed
    void getCurrButtonShownPosition(int &x, int &y);
    void getCurrButtonHiddenPosition(int &x, int &y);

    void paintEvent(QPaintEvent *);
};

} } // end namespaces
