/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QTreeView>
#include <QStandardItemModel>
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include "IrcChanTopicDelegate.h"
#include "IrcParser.h"

IrcChanTopicDelegate::IrcChanTopicDelegate(QObject *parent/* = NULL*/)
    : QAbstractItemDelegate(parent)
{ }

void IrcChanTopicDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // highlight the item
    if(option.state & QStyle::State_Selected)
    {
        QPalette pal = option.palette;
        QBrush brush = pal.highlight();
        //QColor col = brush.color();
        //brush.setColor(col);
        painter->fillRect(option.rect, brush);
    }

    painter->save();

    QTextDocument doc;
    QAbstractTextDocumentLayout::PaintContext context;

    doc.setDefaultFont(m_font);
    doc.setHtml(index.data().toString());
    doc.setPageSize(option.rect.size());
    painter->setClipRect(option.rect);
    painter->translate(option.rect.x(), option.rect.y());
    doc.documentLayout()->draw(painter, context);

    painter->restore();
}

QSize IrcChanTopicDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QTreeView *pView = dynamic_cast<QTreeView *>(parent());
    QStandardItemModel *pModel = dynamic_cast<QStandardItemModel *>(pView->model());
    QSize size = pModel->item(index.row(), 2)->sizeHint();
    size.setWidth(size.width() + 5);
    size.setHeight(size.height() + 5);
    return size;
}
