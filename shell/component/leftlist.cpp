#include "leftlist.h"

#include <QDebug>

LeftList::LeftList(QWidget *parent) : QListWidget(parent)
{

}

void LeftList::resizeEvent(QResizeEvent *event) {
    int maxItemWidth = 0;
    for (int i=0; i<this->count(); i++) {
        QWidget *item = this->itemWidget(this->item(i));
        if (item->width() > maxItemWidth) {
            maxItemWidth = item->width();
        }
    };
    if (maxItemWidth < 170) {
        this->setMinimumWidth( maxItemWidth );
    }
    QListWidget::resizeEvent(event);
}
