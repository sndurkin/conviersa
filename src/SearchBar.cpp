/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QPaintEvent>
#include <QPainter>
#include <QBrush>
#include <QLinearGradient>
#include "SearchBar.h"

namespace cv {

SearchBar::SearchBar(QWidget *pParent/* = NULL*/)
    : CLineEdit("Search", pParent),
      m_searchProgress(0.0)
{
    QLinearGradient *pGrad = new QLinearGradient(0, 0, 0, height());
    QColor edge("powderblue");
    edge.setAlphaF(0.7);
    QColor middle("dodgerblue");
    middle.setAlphaF(0.65);
    pGrad->setColorAt(0.0, edge);
    pGrad->setColorAt(0.3, middle);
    pGrad->setColorAt(0.55, edge);
    m_pBrush = new QBrush(*pGrad);
}

// updates the progress within the search bar
void SearchBar::updateProgress(double newProgress)
{
    m_searchProgress = newProgress;
    update();
}

// clears the progress within the search bar
void SearchBar::clearProgress()
{
    m_searchProgress = 0.0;
    update();
}

// called when the search bar needs to be repainted
void SearchBar::paintEvent(QPaintEvent *event)
{
    CLineEdit::paintEvent(event);
    if(m_searchProgress > 0.0)
    {
        QPainter painter(this);
        // calculate the rect to draw and then draw it
        QRectF currRect(QPointF(1,1), QPointF((width()-1) * m_searchProgress, height()-1));
        painter.fillRect(currRect, *m_pBrush);
    }
}

} // end namespace
