#include "entity.h"

Entity::Entity(QSize size, QPoint pos, Direction dir, int speed, int criticalCnt, QGraphicsScene* scene) : MyItem(size, pos, dir, speed, criticalCnt, scene) {}

Entity::~Entity() {}

bool Entity::movable(const QList<QPoint>& points, Atlas& atlas) {
    bool flag = true;
    for (const QPoint& point : points) {
        if (!QRect(0, 0, 32, 32).contains(point)) {
            flag = false;
        } else {
            switch (atlas.block(point)) {
                case Brick:
                case Steel:
                case River:
                    flag = false;
                    break;
                default:
                    break;
            }
        }
    }
    return flag;
}

bool Entity::blockedBy(const QRectF& rect, const QList<MyItem*>& obstacles) const {
    for (const MyItem* obstacle : obstacles) {
        if (obstacle != this && obstacle->isConnected() && rect.intersects(obstacle->rect())) {
            return true;
        }
    }
    return false;
}

bool Entity::move(Atlas& atlas, const QList<MyItem*>& obstacles) {
    Q_UNUSED(atlas);
    Q_UNUSED(obstacles);
    return m_stateCnt >= m_criticalCnt;
}

void Entity::turnLeft() {
    m_dir = (Direction)((m_dir + 1) % 4);
}

void Entity::turnRight() {
    m_dir = (Direction)((m_dir + 3) % 4);
}

void Entity::turnBack() {
    m_dir = (Direction)(m_dir ^ 2);
}
