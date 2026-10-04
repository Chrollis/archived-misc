#ifndef PLAYER_H
#define PLAYER_H

#include "loot.h"
#include "oritank.h"

class Player : public OriTank {
private:
    int m_backup;
    int m_boostCnt;
    int m_boostCooldown;
    bool m_noCooldown;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

public:
    Player(QPoint pos, Direction dir, int rank, QGraphicsScene* scene = nullptr);
    ~Player();
    bool hit() override;
    bool move(Atlas& atlas, const QList<MyItem*>& obstacles = {}) override;

public:
    void fetchLoot(Loot& loot);
    void recover(QPoint pos, Direction dir, int rank, QGraphicsScene* scene);
    int backup() const;
    int rank() const;
    void setBoost(bool on);
    void setNoCooldown(bool on);
    bool isBoosting() const;
    int boostRemaining() const;
    int boostCooldown() const;
};

#endif  // PLAYER_H
