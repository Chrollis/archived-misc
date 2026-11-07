#ifndef SINGLEGAME_H
#define SINGLEGAME_H

#include "game.h"

class SingleGame : public Game {
public:
    SingleGame(QGraphicsScene* scene);
    ~SingleGame();

    void update();
    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);
};

#endif  // SINGLEGAME_H
