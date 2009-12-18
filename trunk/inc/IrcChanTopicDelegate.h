#ifndef IRCCHANTOPICDELEGATE_H
#define IRCCHANTOPICDELEGATE_H

#include <QAbstractItemDelegate>
#include <QFont>

class IrcChanTopicDelegate : public QAbstractItemDelegate
{
	Q_OBJECT
	
	QFont		m_font;
	
public:
	IrcChanTopicDelegate(QObject *parent = NULL);
	
	void paint(QPainter *painter, const QStyleOptionViewItem &option,
					const QModelIndex &index) const;
	
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	
	void setFont(const QFont &font) { m_font = font; }
};

#endif