#ifndef SNAKE_H
#define SNAKE_H

#include <QGraphicsScene>
#include <QQueue>

#include "entity.h"

class Snake {
private:
    QQueue<Entity*> m_bodies;
    QGraphicsScene* m_scene;

public:
    Snake(QGraphicsScene* scene);
    static bool is_border(const QPoint& pos);
    static bool is_walkable(const QPoint& pos, const QMap<QPoint, Entity*>& occupied);
    bool is_empty();
    Entity* head();
    Entity* tail();
    const QQueue<Entity*>& bodies();
    void update();

    bool if_turn(const QMap<QPoint, Entity*> occupied);
    bool contains(Entity* tar);
    void set_next(Direction next);
    QPoint next_head_position();

    void push_head();
    QPoint pop_tail();
    QMap<QPoint, Entity*> init(const QPoint& start, Direction dir);
    QList<QPoint> clear();
};

#endif  // SNAKE_H
