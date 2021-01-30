#ifndef LEFTLIST_H
#define LEFTLIST_H

#include <QListWidget>
#include <QListWidgetItem>
#include <QResizeEvent>

class LeftList : public QListWidget
{
public:
    LeftList(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event);

};

#endif // LEFTLIST_H
