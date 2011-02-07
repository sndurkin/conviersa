/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QLineEdit>

namespace cv { namespace gui {

// defines a custom line edit class that is basically a QLineEdit
// with some additional features:
//  - custom text is displayed in the lineedit when it does
//  not have focus (and there is no text inside it)
//  - when there is text in the search bar but there is no
//  focus, and the user clicks inside it, it highlights all
//  the text
class CLineEdit : public QLineEdit
{
    QString     m_textToPrint;
    bool        m_printText;
    bool        m_selectAll;

public:
    CLineEdit(const QString &textToPrint, QWidget *pParent = NULL);

    void setPrintedText(const QString &textToPrint) { m_textToPrint = textToPrint; }
    QString getPrintedText() { return m_textToPrint; }

protected:
    // returns the ideal size for the line edit
    QSize sizeHint() const;

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);

    void paintEvent(QPaintEvent *event);
};

} } // end namespaces
