#ifndef COWORKGAME_H
#define COWORKGAME_H

#include "game.h"

class CoworkGame : public Game {
protected:
    Snake* n_snake;

    QGraphicsPixmapItem* text_1p;
    QGraphicsPixmapItem* text_2p;

public:
    CoworkGame(QGraphicsScene* scene);
    ~CoworkGame();

    void update();
    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void n_move(bool grow);
    void text_move();
};

#endif  // COWORKGAME_H
