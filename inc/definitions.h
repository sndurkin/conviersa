#ifndef DEFINITIONS_H
#define DEFINITIONS_H

class QTreeWidgetItem;
class IWindow;

// IWindow types
const size_t TYPE_IWINDOW		= 0;
const size_t TYPE_IRCWINDOW		= 1;
const size_t TYPE_IRCCHANWINDOW 	= 2;
const size_t TYPE_IRCPRIVWINDOW 	= 3;

// for management by the WindowManager class
struct win_info_t
{
	QTreeWidgetItem *	m_pTreeItem;
	IWindow *		m_pWindow;
};

#endif
