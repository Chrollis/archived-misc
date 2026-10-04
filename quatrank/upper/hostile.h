#ifndef HOSTILE_H
#define HOSTILE_H

#include "oritank.h"

class Hostile : public OriTank {
private:
    bool m_flickered;
    int m_flickerCnt;
    int m_engineCnt;
    QList<QPoint> m_path;
    QPointF m_target;
    int m_pathCnt;
    int m_wanderChance;
    int m_wanderCnt;
    Direction m_wanderDir;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;
    void wander(Atlas& atlas, const QList<MyItem*>& obstacles);

public:
    Hostile(QPoint pos, Direction dir, int rank, QGraphicsScene* scene = nullptr);
    ~Hostile();
    bool isFlickered() const;
    void setTarget(const QPointF& pos);
    void setWandering(bool on);
    int wanderChance() const;
    bool move(Atlas& atlas, const QList<MyItem*>& obstacles = {}) override;
};

#endif  // HOSTILE_H
