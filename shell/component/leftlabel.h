#ifndef LEFTLABEL_H
#define LEFTLABEL_H

#include <QLabel>
#include <QFontMetrics>
#include <QEvent>
#include <QResizeEvent>
#include <QString>

class LeftLabel : public QLabel
{
public:
    LeftLabel(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    LeftLabel(const QString &text, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    ~LeftLabel();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    QString mStr;
};

#endif // LEFTLABEL_H
