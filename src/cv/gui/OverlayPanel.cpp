/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QPushButton>
#include <QPainter>
#include <QStyleOption>
#include <QSignalMapper>
#include <QStringBuilder>
#include <QGraphicsDropShadowEffect>
#include "cv/gui/OverlayPanel.h"

namespace cv { namespace gui {

OverlayPanel::OverlayPanel(QWidget *parent)
    : QWidget(parent),

      m_isInitialized(false),
      m_state(OPEN),

      m_alignment(Qt::AlignTop),
      m_secondaryAlignment(0),
      m_firstOffset(-1),
      m_secondOffset(-1),

      m_duration(100),

      m_pCurrOpenButton(NULL),
      m_buttonAlignment(Qt::AlignLeft),
      m_buttonOffset(0),
      m_pOpenBtnAnimation(NULL)
{
    m_pSlideOpenAnimation = new QPropertyAnimation(this, "geometry");
    QObject::connect(m_pSlideOpenAnimation, SIGNAL(finished()), this, SIGNAL(panelOpened()));
    m_pSlideClosedAnimation = new QPropertyAnimation(this, "geometry");
    QObject::connect(m_pSlideClosedAnimation, SIGNAL(finished()), this, SIGNAL(panelClosed()));

    m_pSignalMapper = new QSignalMapper(this);

    QObject::connect(m_pSlideClosedAnimation, SIGNAL(finished()), this, SLOT(onPanelClosed()));
}

//-----------------------------------//

// sets the duration for the OverlayPanel
void OverlayPanel::setDuration(int duration)
{
    if(!m_isInitialized)
        m_duration = duration;
}

//-----------------------------------//

// sets the button alignment and offset
void OverlayPanel::setButtonConfig(Qt::AlignmentFlag btnAlignment, int btnOffset)
{
    if(!m_isInitialized)
    {
        m_buttonAlignment = btnAlignment;
        m_buttonOffset = btnOffset;
    }
}

//-----------------------------------//

// adds an open button for the OverlayPanel
QPushButton *OverlayPanel::addOpenButton(QWidget *pParent, const QString &btnText, int w, int h)
{
    if(m_isInitialized)
    {
        QPushButton *pOpenButton = new QPushButton(pParent);
        pOpenButton->setText(btnText);
        pOpenButton->resize(w, h);

        QString
            btnCss = QString("QPushButton { ") %
                     QString("color: rgba(153, 153, 153, 50%); ") %
                     QString("background-color: transparent; ") %
                     QString("border-width: 1px; ") %
                     QString("border-style: solid; ") %
                     QString("border-color: rgba(153, 153, 153, 50%); ");
        if(m_alignment == Qt::AlignTop)
        {
            btnCss += QString("border-bottom-left-radius: 5px; ") %
                      QString("border-bottom-right-radius: 5px; ") %
                      QString("border-top-width: 0px; } ");
        }
        else if(m_alignment == Qt::AlignBottom)
        {
            btnCss += QString("border-top-left-radius: 5px; ") %
                      QString("border-top-right-radius: 5px; ") %
                      QString("border-bottom-width: 0px; } ");
        }
        else if(m_alignment == Qt::AlignLeft)
        {
            btnCss += QString("border-top-right-radius: 5px; ") %
                      QString("border-bottom-right-radius: 5px; ") %
                      QString("border-left-width: 0px; } ");
        }
        else
        {
            btnCss += QString("border-top-left-radius: 5px; ") %
                      QString("border-bottom-left-radius: 5px; ") %
                      QString("border-right-width: 0px; } ");
        }
            btnCss += QString("QPushButton:hover { ") %
                      QString("color: palette(button-text); ") %
                      QString("border-color: palette(dark); ") %
                      QString("background-color: palette(button); }");
        pOpenButton->setStyleSheet(btnCss);

        QObject::connect(pOpenButton, SIGNAL(clicked()), m_pSignalMapper, SLOT(map()));
        m_pSignalMapper->setMapping(pOpenButton, pOpenButton);
        QObject::connect(m_pSignalMapper, SIGNAL(mapped(QWidget*)), this, SLOT(onOpenClicked(QWidget*)));

        // if this open button shares the same parent as the panel,
        // then create the animation for it
        if(pParent == parentWidget())
        {
            m_pCurrOpenButton = pOpenButton;
            realignPanel(m_pCurrOpenButton);

            int btnShownX, btnShownY, btnHiddenX, btnHiddenY;
            getCurrButtonShownPosition(btnShownX, btnShownY);
            getCurrButtonHiddenPosition(btnHiddenX, btnHiddenY);

            m_pOpenBtnAnimation = new QPropertyAnimation(m_pCurrOpenButton, "geometry");
            m_pOpenBtnAnimation->setStartValue(QRect(btnHiddenX, btnHiddenY,
                                                     m_pCurrOpenButton->width(),
                                                     m_pCurrOpenButton->height()));
            m_pOpenBtnAnimation->setEndValue(QRect(btnShownX, btnShownY,
                                                   m_pCurrOpenButton->width(),
                                                   m_pCurrOpenButton->height()));
        }

        return pOpenButton;
    }

    return NULL;
}

//-----------------------------------//

// sets the alignment for the OverlayPanel
void OverlayPanel::setAlignment(Qt::AlignmentFlag alignment)
{
    if(!m_isInitialized)
        m_alignment = alignment;
}

//-----------------------------------//

// sets the secondary alignment for the OverlayPanel
void OverlayPanel::setSecondaryAlignment(Qt::Alignment alignment, int firstOffset, int secondOffset)
{
    if(!m_isInitialized)
    {
        m_secondaryAlignment = alignment;
        m_firstOffset = firstOffset;
        m_secondOffset = secondOffset;
    }
}

//-----------------------------------//

// sets the initial state of the OverlayPanel
void OverlayPanel::setInitialState(OverlayState state)
{
    if(!m_isInitialized)
        m_state = state;
}

//-----------------------------------//

// initializes everything so that the OverlayPanel can be used; assumes
// that the panel's current x and y are for the OPEN state
void OverlayPanel::initialize()
{
    m_isInitialized = true;

    if(m_state == CLOSED)
    {
        int closedX, closedY;
        if(m_alignment == Qt::AlignTop || m_alignment == Qt::AlignBottom)
        {
            closedX = x();
            closedY = (m_alignment == Qt::AlignTop) ? -height() : parentWidget()->height();
        }
        else
        {
            closedY = y();
            closedX = (m_alignment == Qt::AlignLeft) ? -width() : parentWidget()->width();
        }
        move(closedX, closedY);
    }
}

//-----------------------------------//

void OverlayPanel::open(bool animate/* = true*/)
{
    if(m_state != OPEN && m_isInitialized)
    {
        // hide the open button, if it exists
        if(m_pCurrOpenButton != NULL)
        {
            int btnClosedX, btnClosedY;
            getCurrButtonHiddenPosition(btnClosedX, btnClosedY);
            m_pCurrOpenButton->move(btnClosedX, btnClosedY);
        }

        int openX, openY;
        if(m_alignment == Qt::AlignTop || m_alignment == Qt::AlignBottom)
        {
            openX = x();
            openY = (m_alignment == Qt::AlignTop) ? 0 : (parentWidget()->height() - height());
        }
        else
        {
            openY = y();
            openX = (m_alignment == Qt::AlignLeft) ? 0 : (parentWidget()->width() - width());
        }

        if(animate)
        {
            QRect openRect(openX, openY, width(), height());
            QRect closedRect(x(), y(), width(), height());

            m_pSlideOpenAnimation->setDuration(m_duration);
            m_pSlideOpenAnimation->setStartValue(closedRect);
            m_pSlideOpenAnimation->setEndValue(openRect);

            m_pSlideOpenAnimation->start();
        }
        else
        {
            move(openX, openY);
            emit panelOpened();
        }

        m_state = OPEN;
    }
}

//-----------------------------------//

void OverlayPanel::close(bool animate/* = true*/)
{
    if(m_state != CLOSED && m_isInitialized)
    {
        int closedX, closedY;
        if(m_alignment == Qt::AlignTop || m_alignment == Qt::AlignBottom)
        {
            closedX = x();
            closedY = (m_alignment == Qt::AlignTop) ? -height() : parentWidget()->height();
        }
        else
        {
            closedY = y();
            closedX = (m_alignment == Qt::AlignLeft) ? -width() : parentWidget()->width();
        }

        if(animate)
        {
            QRect openRect(x(), y(), width(), height());
            QRect closedRect(closedX, closedY, width(), height());

            m_pSlideClosedAnimation->setDuration(m_duration);
            m_pSlideClosedAnimation->setStartValue(openRect);
            m_pSlideClosedAnimation->setEndValue(closedRect);

            // reset the animation for the open button
            if(m_pCurrOpenButton != NULL)
            {
                int btnShownX, btnShownY, btnHiddenX, btnHiddenY;
                getCurrButtonShownPosition(btnShownX, btnShownY);
                getCurrButtonHiddenPosition(btnHiddenX, btnHiddenY);
                m_pOpenBtnAnimation->setTargetObject(m_pCurrOpenButton);
                m_pOpenBtnAnimation->setDuration(m_duration);
                m_pOpenBtnAnimation->setStartValue(QRect(btnHiddenX, btnHiddenY,
                                                         m_pCurrOpenButton->width(),
                                                         m_pCurrOpenButton->height()));
                m_pOpenBtnAnimation->setEndValue(QRect(btnShownX, btnShownY,
                                                       m_pCurrOpenButton->width(),
                                                       m_pCurrOpenButton->height()));
            }

            m_pSlideClosedAnimation->start();
        }
        else
        {
            move(closedX, closedY);
            if(m_pCurrOpenButton != NULL)
            {
                int btnOpenX, btnOpenY;
                getCurrButtonShownPosition(btnOpenX, btnOpenY);
                m_pCurrOpenButton->move(btnOpenX, btnOpenY);
            }
        }
        m_state = CLOSED;
    }
}

//-----------------------------------//

void OverlayPanel::toggle(bool animate/* = true*/)
{
    if(m_state == OPEN)
        close(animate);
    else
        open(animate);
}

//-----------------------------------//

void OverlayPanel::onOpenClicked(QWidget *pButton)
{
    // if the button clicked is not inside the same widget
    // as the panel
    if(parent() != pButton->parentWidget())
    {
        // make sure the panel is closed (so the button
        // will be shown)
        close(false);

        // reparent the panel to this button's parent
        setParent(pButton->parentWidget());
        show();

        // set the current open button
        m_pCurrOpenButton = dynamic_cast<QPushButton *>(pButton);
        realignPanel(m_pCurrOpenButton);
    }

    // open the panel
    open();
}

//-----------------------------------//

void OverlayPanel::onPanelClosed()
{
    if(m_pOpenBtnAnimation != NULL)
        m_pOpenBtnAnimation->start();
}

//-----------------------------------//

// realigns the OverlayPanel to the parent widget
void OverlayPanel::realignPanel(QPushButton *pOpenButton/* = NULL*/)
{
    // update the position for the panel (and, if applicable, the open button)
    // according to the alignment rules
    int panelX = x(), panelY = y(), panelW = width(), panelH = height();
    if(m_alignment == Qt::AlignTop || m_alignment == Qt::AlignBottom)
    {
        if(m_state == OPEN)
            panelY = (m_alignment == Qt::AlignTop) ? 0 : (pOpenButton->parentWidget()->height() - height());
        else
            panelY = (m_alignment == Qt::AlignTop) ? -height() : pOpenButton->parentWidget()->height();

        if(m_secondaryAlignment & Qt::AlignLeft)
        {
            // set the x of the panel
            panelX = m_firstOffset;

            // also set the width of the panel
            if(m_secondaryAlignment & Qt::AlignRight)
                panelW = pOpenButton->parentWidget()->width() - (m_firstOffset + m_secondOffset);
        }
        else if(m_secondaryAlignment & Qt::AlignRight)
        {
            // set the x of the panel
            panelX = pOpenButton->parentWidget()->width() - (width() + m_firstOffset);
        }
    }
    else // aligned to the left or right
    {
        if(m_state == OPEN)
            panelX = (m_alignment == Qt::AlignTop) ? 0 : (pOpenButton->parentWidget()->width() - width());
        else
            panelX = (m_alignment == Qt::AlignTop) ? -width() : pOpenButton->parentWidget()->width();

        if(m_secondaryAlignment & Qt::AlignTop)
        {
            // set the y of the panel
            panelY = m_firstOffset;

            // also set the height of the panel
            if(m_secondaryAlignment & Qt::AlignBottom)
                panelH = pOpenButton->parentWidget()->height() - (m_firstOffset + m_secondOffset);
        }
        else if(m_secondaryAlignment & Qt::AlignBottom)
        {
            // set the y of the panel
            panelY = pOpenButton->parentWidget()->height() - (height() + m_firstOffset);
        }
    }

    if(pOpenButton != NULL)
    {
        // move and resize the panel if shares the same parent as
        // the button
        if(m_pCurrOpenButton == pOpenButton)
        {
            move(panelX, panelY);
            resize(panelW, panelH);
        }

        realignButton(pOpenButton, panelX, panelY);
    }
}

//-----------------------------------//

// find the offsets for the button, and move it
void OverlayPanel::realignButton(QPushButton *pOpenButton, int x, int y)
{
    if(pOpenButton != NULL)
    {
        int btnX, btnY;
        if(m_alignment == Qt::AlignTop || m_alignment == Qt::AlignBottom)
        {
            if(m_buttonAlignment == Qt::AlignLeft)
                btnX = x + m_buttonOffset;
            else
                btnX = x + width() - (pOpenButton->width() + m_buttonOffset);

            if(m_state == OPEN && m_pCurrOpenButton == pOpenButton)
                btnY = (m_alignment == Qt::AlignTop) ? -pOpenButton->height() : pOpenButton->parentWidget()->height();
            else
                btnY = (m_alignment == Qt::AlignTop) ? 0 : (pOpenButton->parentWidget()->height() - pOpenButton->height());
        }
        else
        {
            if(m_buttonAlignment == Qt::AlignTop)
                btnY = y + m_buttonOffset;
            else
                btnY = y + height() - (pOpenButton->height() + m_buttonOffset);

            if(m_state == OPEN && m_pCurrOpenButton == pOpenButton)
                btnX = (m_alignment == Qt::AlignLeft) ? -pOpenButton->width() : pOpenButton->parentWidget()->width();
            else
                btnX = (m_alignment == Qt::AlignLeft) ? 0 : (pOpenButton->parentWidget()->width() - pOpenButton->width());
        }

        pOpenButton->move(btnX, btnY);
    }
}

//-----------------------------------//

// retrieve button position for when it's open
void OverlayPanel::getCurrButtonShownPosition(int &x, int &y)
{
    x = m_pCurrOpenButton->x();
    y = m_pCurrOpenButton->y();

    if(m_state == OPEN)
    {
        if(m_alignment == Qt::AlignTop)
            y += m_pCurrOpenButton->height();
        else if(m_alignment == Qt::AlignBottom)
            y -= m_pCurrOpenButton->height();
        else if(m_alignment == Qt::AlignLeft)
            x += m_pCurrOpenButton->width();
        else
            x -= m_pCurrOpenButton->width();
    }
}

//-----------------------------------//

// retrieve button position for when it's closed
void OverlayPanel::getCurrButtonHiddenPosition(int &x, int &y)
{
    x = m_pCurrOpenButton->x();
    y = m_pCurrOpenButton->y();

    if(m_state == CLOSED)
    {
        if(m_alignment == Qt::AlignTop)
            y = -m_pCurrOpenButton->height();
        else if(m_alignment == Qt::AlignBottom)
            y = parentWidget()->height();
        else if(m_alignment == Qt::AlignLeft)
            x = -m_pCurrOpenButton->width();
        else
            x = parentWidget()->width();
    }
}

//-----------------------------------//

void OverlayPanel::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

} } // end namespaces
