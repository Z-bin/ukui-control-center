#include "itemdelegate.h"
#include <QDebug>

ItemDelegate::ItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    QStyledItemDelegate::paint(painter,option,index);

    QString title = index.data(Qt::UserRole).toString();
    QFont f = option.font;
    painter->setFont(f);
    QFontMetrics fm(f);

    QRect r = option.rect;
    // r = r.adjusted(0, fm.lineSpacing(), 0, 0);
    painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft|Qt::TextWordWrap, title, &r);

    painter->restore();
}

//QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//    QFont f = option.font;
//    QRect r = option.rect;
//    QFontMetrics fm(f);
//    QString title = index.data(Qt::UserRole).toString();
//    qDebug() << "this is --->" << title << r.size();
//    QRect br = fm.boundingRect(r,Qt::AlignTop|Qt::AlignLeft | Qt::TextWordWrap,title);
//    return QSize(option.rect.width(),40);
//}
