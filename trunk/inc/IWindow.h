/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QMdiSubWindow>

namespace cv {

class WindowManager;
class WindowContainer;

class IWindow : public QWidget
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

    // we need this pointer so every IWindow instance has the ability
    // to change other IWindow instances (through the WindowManager)
    WindowManager *     m_pManager;

    // we need this so we can put our IWindow inside a WindowContainer
    // (it has to be inside a QMdiSubWindow so everything will work right)
    QMdiSubWindow *     m_pSubWindow;

public:
    IWindow(const QString &title = tr("Untitled"),
            const QSize &size = QSize(500, 300));
    virtual ~IWindow() { }

    QSize sizeHint() const;

    // returns the title of the window, whether it's
    // in a container or on the desktop
    QString getTitle() const;

    // sets the title of the window, whether it's
    // in a container or on the desktop
    void setTitle(const QString &title);

    // the name returned is the short name displayed in the WindowManager
    QString getWindowName();

    // sets the short name of the window (displayed in the WM)
    void setWindowName(const QString &name);

    bool hasContainer() const { return m_pContainer != NULL; }

    virtual void giveFocus() = 0;

protected:
    virtual void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);
};

} // end namespace
