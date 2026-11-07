#ifndef ENTITY_H
#define ENTITY_H

#include <QGraphicsItem>
#include <QPainter>
#include <QPixmap>

#define gridSize 32
#define gridEdge 16
#define snakeSize 24
#define tomatoSize 16

enum EntityType { Clean, Head, Body, Tail, Tomato, Domado, Stone };
enum Direction { Up, Left, Down, Right };

class Entity : public QGraphicsItem {
public:
    Entity(EntityType type, QGraphicsItem* parent = nullptr);
    Entity(const Entity& other);

    void set_position(QPoint pos);
    QPoint position() const;
    void set_direction(Direction this_dir);
    Direction diretion() const;
    void set_next(Direction next_dir);
    Direction next() const;
    void set_type(EntityType type);
    EntityType get_type() const;
    void add_life_counter();

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

private:
    EntityType m_type = Tomato;
    QPoint m_position = {0, 0};
    Direction m_direction = Right;
    Direction n_direction = Right;
    int life_counter = 0;
};

bool operator<(const QPoint& a, const QPoint& b);

#endif  // ENTITY_H
