#ifndef MYITEM_H
#define MYITEM_H

#include <QGraphicsItem>
#include <QGraphicsScene>
#include "utils.h"

class MyItem : public QGraphicsItem {
protected:
    QGraphicsScene* m_scene;
    QSize m_size;
    Direction m_dir;
    int m_speed;
    int m_stateCnt;

protected:
    const int m_criticalCnt;
    QList<QPoint> premove(QRectF& rect, Direction dir);

public:
    MyItem(QSize size, QPoint pos, Direction dir, int speed, int criticalCnt, QGraphicsScene* scene = nullptr);
    virtual ~MyItem();
    void reconnect(QGraphicsScene* scene);
    void disconnect();

public:
    virtual bool bomb();
    void setDir(Direction dir);

public:
    QRectF boundingRect() const override;
    QRectF rect() const;
    Direction dir() const;
    bool isConnected() const;
    bool isInteractable() const;
};

#endif  // MYITEM_H
