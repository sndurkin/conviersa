/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QMdiArea>
#include <QMdiSubWindow>

namespace cv { namespace gui {

class Window;

// custom QMdiArea that i'm using as a container
// to hold Windows
class WindowContainer : public QMdiArea
{
protected:
    QList<Window *> m_windows;

public:
    WindowContainer(QWidget* pParent);
    virtual ~WindowContainer() { }

    int size() const { return m_windows.size(); }
};

} } // end namespaces
