#ifndef GAME_H
#define GAME_H

#include <QKeyEvent>
#include <QMap>
#include <QMouseEvent>
#include <QRandomGenerator>

#include "snake.h"

class Game {
protected:
    int m_score = 0;
    bool over = 0;
    bool up = 0;
    QGraphicsScene* m_scene;
    Snake* m_snake;
    QMap<QPoint, Entity*> m_occupied;

public:
    Game(QGraphicsScene* scene);
    virtual ~Game();

    int score();
    void create_tomato();
    void move(bool grow);
    void kill(Snake* snake);
    bool is_over();
    bool is_up();
    void update_tomato();
    virtual void update() = 0;
    virtual void keyPressEvent(QKeyEvent* event) = 0;
    virtual void mousePressEvent(QMouseEvent* event) = 0;
};

#endif  // GAME_H
