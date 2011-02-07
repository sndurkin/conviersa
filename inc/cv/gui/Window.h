/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QMdiSubWindow>

namespace cv { namespace gui {

class WindowManager;
class WindowContainer;

class Window : public QWidget
{
    friend class WindowManager;
    friend class WindowContainer;

protected:
    // we need a pointer to the current container instance
    // to be able to destroy it when there are no more
    // tabs in a desktop window
    WindowContainer *   m_pContainer;

    // this is used to maintain the default size of the window
    QSize               m_defSize;

    // we need this pointer so every Window instance has the ability
    // to change other Window instances (through the WindowManager)
    WindowManager *     m_pManager;

    // we need this so we can put our Window inside a WindowContainer
    // (it has to be inside a QMdiSubWindow so everything will work right)
    QMdiSubWindow *     m_pSubWindow;

public:
    Window(const QString &title = tr("Untitled"),
            const QSize &size = QSize(500, 300));
    virtual ~Window() { }

    QSize sizeHint() const;

    QString getTitle() const;
    void setTitle(const QString &title);

    QString getWindowName();
    void setWindowName(const QString &name);

    bool hasContainer() const { return m_pContainer != NULL; }

    // called when the user clicks the corresponding item in the WindowManager
    virtual void giveFocus() = 0;

    // called when the corresponding item in the WindowManager becomes focused
    virtual void focusedInTree() { }

protected:
    virtual void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);
};

} } // end namespaces
