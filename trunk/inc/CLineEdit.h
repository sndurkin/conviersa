/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QLineEdit>

// defines a custom line edit class that is basically a QLineEdit
// with some additional features:
// 	- custom text is displayed in the lineedit when it does
//	not have focus (and there is no text inside it)
//	- when there is text in the search bar but there is no
//	focus, and the user clicks inside it, it highlights all
//	the text
class CLineEdit : public QLineEdit
{
	QString	m_textToPrint;
	bool		m_printText;
	bool		m_selectAll;

public:
	CLineEdit(const QString &textToPrint, QWidget *pParent = NULL);

	void SetPrintedText(const QString &textToPrint) { m_textToPrint = textToPrint; }
	QString GetPrintedText() { return m_textToPrint; }
	
protected:
	// returns the ideal size for the line edit
	QSize sizeHint() const;
	
	// imitate firefox search bar behavior
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	
	// when the line edit gets focus
	void focusInEvent(QFocusEvent *event);
	
	// when the line edit loses focus
	void focusOutEvent(QFocusEvent *event);
	
	// called when the line edit needs to be repainted
	void paintEvent(QPaintEvent *event);
};
