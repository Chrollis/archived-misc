#ifndef PREGAME_H
#define PREGAME_H

#include "game.h"

struct Node {
    QPoint pos;
    int G;
    int H;
    Node* parent;

    Node(QPoint pos, QPoint tar, Snake* snake = nullptr);
    Node(Node* parent, QPoint dir, QPoint tar, Snake* snake = nullptr);
    int penalty(Snake* snake = nullptr);
    int F();
};
bool operator==(const Node& a, const Node& b);

class PreGame : public Game {
private:
    QPoint m_target = {0, 0};
    QList<Direction> direction_list;
    QGraphicsRectItem* rect;
    int retarget_counter = 0;

public:
    PreGame(QGraphicsScene* scene);
    ~PreGame();

    void set_target();
    QList<Direction> find_path(const QPoint& tomato_pos);
    void update();
    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);
};

#endif  // PREGAME_H
