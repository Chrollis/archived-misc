#ifndef BULLET_H
#define BULLET_H

#include "entity.h"

class Bullet : public Entity {
private:
    MyItem* m_maker;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

public:
    Bullet(QPoint pos, Direction dir, int speed, MyItem* maker = nullptr, QGraphicsScene* scene = nullptr);
    ~Bullet();
    const MyItem* const maker() const;
    bool move(Atlas& atlas, const QList<MyItem*>& obstacles = {}) override;
    bool movable(const QList<QPoint>& points, Atlas& atlas) override;
};

#endif  // BULLET_H
