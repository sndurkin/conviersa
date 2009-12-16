#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include "CLineEdit.h"

class QFocusEvent;
class QPaintEvent;
class QBrush;

class SearchBar : public CLineEdit
{
	bool		m_printText;
	bool		m_selectAll;
	
	QBrush *	m_pBrush;
	
	// value is between 0.0 and 1.0
	double	m_searchProgress;

public:
	SearchBar(QWidget *pParent = NULL);
	
	// updates the progress within the search bar
	void UpdateProgress(double newProgress);
	
	// clears the progress within the search bar
	void ClearProgress();

protected:
	// called when the search bar needs to be repainted
	void paintEvent(QPaintEvent *event);
};

#endif
