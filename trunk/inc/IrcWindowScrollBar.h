#ifndef IRCWINDOWSCROLLBAR_H
#define IRCWINDOWSCROLLBAR_H

#include <QScrollBar>
#include <QStyle>

class IrcWindowScrollBar : public QScrollBar
{
	Q_OBJECT
	
	int	m_currMax;
	
	// stores whether or not the scrollbar has default
	// scrolling functionality
	bool 			m_defaultBehavior;
	
	// stores the list of lines to be painted
	QList<QLineF>	m_searchLines;
	
public:
	IrcWindowScrollBar(QWidget *pParent = NULL);
	
	int GetSliderHeight();
	
	// adds a line to be painted, located at sliderVal,
	// and repaints with the new line
	void AddLine(qreal posRatio);
	
	// clears all the lines, and repaints
	void ClearLines();
	
	void SetDefaultBehavior(bool db) { m_defaultBehavior = db; }
	
	bool GetDefaultBehavior() { return m_defaultBehavior; }
	
protected:
	void mousePressEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);

public slots:
	// ensures that the area holding the scrollbar moves it so that
	// the bottom of the viewport doesn't move
	void UpdateScrollBar(int min, int max);
};

#endif
