/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include "cv/gui/CLineEdit.h"

class QFocusEvent;
class QPaintEvent;
class QBrush;

namespace cv { namespace gui {

class SearchBar : public CLineEdit
{
    bool        m_printText;
    bool        m_selectAll;

    QBrush *    m_pBrush;

    // value is between 0.0 and 1.0
    double      m_searchProgress;

public:
    SearchBar(QWidget *pParent = NULL);

    void updateProgress(double newProgress);
    void clearProgress();

protected:
    void paintEvent(QPaintEvent *event);
};

} } // end namespaces
