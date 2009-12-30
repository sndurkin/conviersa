/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

class QTreeWidgetItem;

namespace cv {

class IWindow;

// IWindow types
enum
{
    TYPE_IWINDOW,
    TYPE_IRCWINDOW,
    TYPE_IRCCHANWINDOW,
    TYPE_IRCPRIVWINDOW
};


// for management by the WindowManager class
struct win_info_t
{
    QTreeWidgetItem *   m_pTreeItem;
    IWindow *           m_pWindow;
};

} // end namespace
