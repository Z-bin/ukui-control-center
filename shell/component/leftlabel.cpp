#include "leftlabel.h"

#include <QDebug>
LeftLabel::LeftLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{

}

LeftLabel::LeftLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{
    mStr = text;
    //    this->setText(text);
}

LeftLabel::~LeftLabel() {
    qDebug() << Q_FUNC_INFO;
}

void LeftLabel::resizeEvent(QResizeEvent *event) {
    qDebug() << Q_FUNC_INFO;
    if (event->type() == QEvent::Resize) {
        QFontMetrics  fontMetrics(this->font());
        int fontSize = fontMetrics.width(mStr);
        if (fontSize > this->width()) {
            this->setText(fontMetrics.elidedText(mStr, Qt::ElideRight, this->width()));
            this->setToolTip("ddddddddddddd");
        } else {
            this->setText(mStr);
            this->setToolTip("");
        }
    }
    QLabel::resizeEvent(event);


    //    if (event->type() == QEvent::Resize) {
    //        if (textLabel) {
    //            QFontMetrics  fontMetrics(textLabel->font());
    //            int fontSize = fontMetrics.width(mStr);
    //            qDebug() << Q_FUNC_INFO << fontSize << textLabel->width() << mStr;
    //            if (fontSize > textLabel->width()) {
    //                textLabel->setText(fontMetrics.elidedText(mStr, Qt::ElideRight, textLabel->width()));
    //                textLabel->setToolTip(mStr);
    //            } else {
    //                textLabel->setText(mStr);
    //                textLabel->setToolTip("");
    //            }
    //        }

    //    }
}

