#ifndef PVPGAME_H
#define PVPGAME_H

#include "coworkgame.h"

class PVPGame : public CoworkGame {
private:
    int n_score = 0;

public:
    PVPGame(QGraphicsScene* scene);
    ~PVPGame();
    int score();

    void update();
};

#endif  // PVPGAME_H
