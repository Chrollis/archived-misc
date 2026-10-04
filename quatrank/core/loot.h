#ifndef LOOT_H
#define LOOT_H

#include "entity.h"

class Loot : public Entity {
private:
    LootType m_category;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

public:
    Loot(QPoint pos, Direction dir, int speed, LootType category, QGraphicsScene* scene = nullptr);
    ~Loot();
    LootType category() const;
    bool move(Atlas& atlas, const QList<MyItem*>& obstacles = {}) override;
};

#endif  // LOOT_H
