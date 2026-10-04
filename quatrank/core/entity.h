#ifndef ENTITY_H
#define ENTITY_H

#include "atlas.h"

class Entity : public MyItem {
public:
    Entity(QSize size, QPoint pos, Direction dir, int speed, int criticalCnt, QGraphicsScene* scene);
    virtual ~Entity();
    virtual bool movable(const QList<QPoint>& points, Atlas& atlas);
    virtual bool move(Atlas& atlas, const QList<MyItem*>& obstacles = {});

protected:
    bool blockedBy(const QRectF& rect, const QList<MyItem*>& obstacles) const;

public:
    void turnLeft();
    void turnRight();
    void turnBack();
};

#endif  // ENTITY_H
