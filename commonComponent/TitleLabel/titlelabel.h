#ifndef TITLELABEL_H
#define TITLELABEL_H

#include <QLabel>
#include <QWidget>

class TitleLabel : public QWidget
{
    Q_OBJECT
public:
    TitleLabel(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    TitleLabel(const QString &text, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

protected:
//    bool event(QEvent *e) override;

public:
    QLabel *mTitleLabel;
};

#endif // TITLELABEL_H
