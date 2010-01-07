/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QFocusEvent>
#include <QMouseEvent>
#include <QPainter>
#include "cv/CLineEdit.h"

namespace cv {

CLineEdit::CLineEdit(const QString &textToPrint, QWidget *pParent/* = NULL*/)
    : QLineEdit(pParent),
      m_textToPrint(textToPrint),
      m_printText(true)
{ }

// returns the ideal size for the line edit
QSize CLineEdit::sizeHint() const
{
    return QSize(150, 20);
}


// imitate firefox search bar behavior
void CLineEdit::mousePressEvent(QMouseEvent *event)
{
    if(text().size() == selectedText().size())
    {
        deselect();
    }

    QLineEdit::mousePressEvent(event);
}

void CLineEdit::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_selectAll && !hasSelectedText())
    {
        selectAll();
    }
    else
    {
        QLineEdit::mouseReleaseEvent(event);
    }

    m_selectAll = false;
}

// when the search bar gets focus
void CLineEdit::focusInEvent(QFocusEvent *event)
{
    if(!hasSelectedText())
    {
        m_selectAll = true;
    }
    m_printText = false;
    QLineEdit::focusInEvent(event);
}

// when the line edit loses focus
void CLineEdit::focusOutEvent(QFocusEvent *event)
{
    m_printText = true;
    m_selectAll = false;
    if(text().size() > 0)
    {
        QString strText = text();
        int i = 0;
        for(; i < strText.size(); ++i)
        {
            if(!strText[i].isSpace())
            {
                m_printText = false;
                break;
            }
        }

        if(i == strText.size())
        {
            // every character in the search bar is
            // whitespace, so clear it out
            clear();
        }
    }

    QLineEdit::focusOutEvent(event);
}

// called when the line edit needs to be repainted
void CLineEdit::paintEvent(QPaintEvent *event)
{
    QLineEdit::paintEvent(event);
    if(m_printText)
    {
        QPainter painter(this);
        QColor color("gray");
        painter.setPen(color);

        painter.drawText(QPointF(5, height()-6), m_textToPrint);
    }
}

} // end namespace
