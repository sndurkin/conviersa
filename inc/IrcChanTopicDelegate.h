/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QAbstractItemDelegate>
#include <QFont>

namespace cv { namespace irc {

class IrcChanTopicDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

    QFont m_font;

public:
    IrcChanTopicDelegate(QObject *parent = NULL);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                    const QModelIndex &index) const;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setFont(const QFont &font) { m_font = font; }
};

} } // end namespaces
