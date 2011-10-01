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
    // if both offsets are specified, then any width (or height, depending
    // on the secondary alignment) for the panel is ignored
    Qt::Alignment           m_secondaryAlignment;
    int                     m_firstOffset,
                            m_secondOffset;

    int                     m_dropShadowWidth;
    Qt::Alignment           m_dropShadowSides;

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
    // but an open button is placed inside each widget; m_pcurrOpenButton
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

    OverlayState getCurrentState() { return m_state; }

    void realignPanel(QPushButton *pOpenButton = NULL);
    QPushButton *addOpenButton(QWidget *pParent, const QString &btnText, int w, int h);

    // open/close functions for panel
    void open(bool animate = true);
    void close(bool animate = true);
    void toggle(bool animate = true);
    bool isOpen(QWidget *pParent) { return (m_state == OPEN && pParent == parentWidget()); }

signals:
    void panelOpened();
    void panelClosed();

public slots:
    void onOpenClicked(QWidget *pButton);
    void onPanelClosed();

protected:
    void setDuration(int duration);
    void setDropShadowConfig(int dropShadowWidth, Qt::Alignment dropShadowSides = (Qt::AlignBottom | Qt::AlignRight));
    void setButtonConfig(Qt::AlignmentFlag btnAlignment, int btnOffset);
    void setAlignment(Qt::AlignmentFlag alignment);
    void setSecondaryAlignment(Qt::Alignment alignment, int firstOffset, int secondOffset);
    void setInitialState(OverlayState state);
    void initialize();

    void realignButton(QPushButton *pButton, int x, int y);

    void getCurrButtonShownPosition(int &x, int &y);
    void getCurrButtonHiddenPosition(int &x, int &y);

    void paintEvent(QPaintEvent *);
};

} } // end namespaces
