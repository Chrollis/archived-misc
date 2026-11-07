#ifndef ORITANK_H
#define ORITANK_H

#include "bullet.h"

class OriTank : public Entity {
protected:
    int m_rank;
    int m_heart;
    int m_shootCnt;
    int m_bulletSpeed;
    int m_maxBulletAmt;
    bool m_engine;
    bool m_infiniteAmmo;
    QList<Bullet*> m_bullets;

public:
    OriTank(QPoint pos, Direction dir, int speed, int rank, QGraphicsScene* scene = nullptr);
    virtual ~OriTank();
    virtual bool hit();
    bool fire(QList<Bullet*>& allBullets);
    bool bomb() override;
    void checkBullets();

public:
    void setEngine(bool engine);
    void setDirection(Direction dir);
    void setInfiniteAmmo(bool on);
};

#endif  // ORITANK_H
