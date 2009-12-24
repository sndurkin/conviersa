/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include "IrcWindowScrollBar.h"
#include <QMouseEvent>
#include <QStyleOptionSlider>
#include <QPainter>

IrcWindowScrollBar::IrcWindowScrollBar(QWidget *pParent/* = NULL*/)
    : QScrollBar(pParent),
      m_defaultBehavior(false)
{
    m_currMax = maximum();
    QObject::connect(this, SIGNAL(rangeChanged(int, int)), this, SLOT(updateScrollBar(int, int)));
}

int IrcWindowScrollBar::getSliderHeight()
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect sr = style()->subControlRect(QStyle::CC_ScrollBar, &opt,
                                        QStyle::SC_ScrollBarSlider, this);

    return sr.height();
}

// adds a line to be painted, located at sliderVal,
// and repaints with the new line
void IrcWindowScrollBar::addLine(qreal posRatio)
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect gr = style()->subControlRect(QStyle::CC_ScrollBar, &opt,
                                    QStyle::SC_ScrollBarGroove, this);

    int posY = (int) (posRatio * gr.height()) + gr.y();

    // now create the line and add it
    QLineF line(0, posY, gr.width(), posY);
    m_searchLines.append(line);

    // calls paintEvent()
    update();
}

// clears all the lines, and repaints
void IrcWindowScrollBar::clearLines()
{
    m_searchLines.clear();
    update();
}

void IrcWindowScrollBar::mousePressEvent(QMouseEvent *event)
{
    if(!m_defaultBehavior && event->button() == Qt::LeftButton)
    {
        // took all this from QScrollBar.cpp
        QStyleOptionSlider opt;
        initStyleOption(&opt);
        QRect gr = style()->subControlRect(QStyle::CC_ScrollBar, &opt,
                                        QStyle::SC_ScrollBarGroove, this);
          QRect sr = style()->subControlRect(QStyle::CC_ScrollBar, &opt,
                                              QStyle::SC_ScrollBarSlider, this);
        int sliderMin, sliderMax, sliderLength;
        sliderLength = sr.height();
        sliderMin = gr.y();
        sliderMax = gr.bottom() - sliderLength + 1;

        QStyle::SubControl sc = style()->hitTestComplexControl(QStyle::CC_ScrollBar, &opt, event->pos(), this);
        if(event->y() >= sliderMin && event->y() <= gr.bottom() && sc != QStyle::SC_ScrollBarSlider)
        {
            int val = QStyle::sliderValueFromPosition(minimum(), maximum(), event->y() - sliderMin - (sliderLength / 2),
                                sliderMax - sliderMin, opt.upsideDown);

            setSliderPosition(val);
        }
        else
        {
            // default action
            QScrollBar::mousePressEvent(event);
        }
    }
    else
    {
        // default action
        QScrollBar::mousePressEvent(event);
    }
}

void IrcWindowScrollBar::paintEvent(QPaintEvent *event)
{
    // paint the default scrollbar first
    QScrollBar::paintEvent(event);

    // now paint all search lines
    QPainter painter(this);
    QColor color("red");
    painter.setPen(color);

    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QStyle::SubControl sc;
    for(int i = 0; i < m_searchLines.size(); ++i)
    {
        sc = style()->hitTestComplexControl(QStyle::CC_ScrollBar, &opt, m_searchLines[i].p1().toPoint(), this);
        if(sc != QStyle::SC_ScrollBarSlider)
            painter.drawLine(m_searchLines[i]);
    }
}

// ensures that the area holding the scrollbar moves it so that
// the bottom of the viewport doesn't move
void IrcWindowScrollBar::updateScrollBar(int min, int max)
{
    setSliderPosition(max - (m_currMax - value()));
    m_currMax = max;
}
