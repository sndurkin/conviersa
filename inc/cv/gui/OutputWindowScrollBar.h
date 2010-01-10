/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QScrollBar>
#include <QStyle>

namespace cv { namespace gui {

class OutputWindowScrollBar : public QScrollBar
{
    Q_OBJECT

    int             m_currMax;

    // stores whether or not the scrollbar has default
    // scrolling functionality
    bool            m_defaultBehavior;

    // stores the list of lines to be painted
    QList<QLineF>   m_searchLines;

public:
    OutputWindowScrollBar(QWidget *pParent = NULL);

    int getSliderHeight();

    // adds a line to be painted, located at sliderVal,
    // and repaints with the new line
    void addLine(qreal posRatio);

    // clears all the lines, and repaints
    void clearLines();

    void setDefaultBehavior(bool db) { m_defaultBehavior = db; }

    bool getDefaultBehavior() { return m_defaultBehavior; }

protected:
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

public slots:
    // ensures that the area holding the scrollbar moves it so that
    // the bottom of the viewport doesn't move
    void updateScrollBar(int min, int max);
};

} } // end namespaces
