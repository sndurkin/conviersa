#ifndef DEFINITIONS_H
#define DEFINITIONS_H

class QTreeWidgetItem;
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
	QTreeWidgetItem *	m_pTreeItem;
	IWindow *		m_pWindow;
};

#endif
