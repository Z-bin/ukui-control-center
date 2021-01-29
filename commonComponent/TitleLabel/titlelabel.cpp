#include "titlelabel.h"

#include <QFont>
#include <QEvent>
#include <QDebug>
#include <QHBoxLayout>
#include <QApplication>

TitleLabel::TitleLabel(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    auto tf = this->font();
    tf.setWeight(QFont::Medium);
    setFont(tf);
}

TitleLabel::TitleLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : TitleLabel(parent, f)
{

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    mTitleLabel = new QLabel(text,this);
    QFont font = this->font();
    auto tf = this->font();
    tf.setWeight(QFont::Medium);
    mTitleLabel->setFont(font);
    layout->addWidget(mTitleLabel);
    this->setLayout(layout);
}

//bool TitleLabel::event(QEvent *e)
//{
//    if (e->type() == QEvent::ApplicationFontChange) {
//        qDebug() << Q_FUNC_INFO;
//        auto tf = this->font();
//        tf.setWeight(QFont::Medium);
//        tf.setBold(t);
//        mTitleLabel->setFont(tf);
//    }

//    return QWidget::event(e);
//}
