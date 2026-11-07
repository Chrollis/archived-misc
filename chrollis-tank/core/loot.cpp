#include "loot.h"

void Loot::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    if (m_stateCnt > 0) {
        if ((m_stateCnt / (2 * FramesPerPicture)) % 2) {
            painter->drawImage(-16, -16, whiteSlashTexture(imgLoot[m_category]));
        } else {
            painter->drawImage(-16, -16, imgLoot[m_category]);
        }
        m_stateCnt -= 1;
    } else {
        disconnect();
    }
}

Loot::Loot(QPoint pos, Direction dir, int speed, LootType category, QGraphicsScene* scene) : Entity(QSize(32, 32), pos, dir, speed, 5 * FramesPerSecond, scene), m_category(category) {
    setZValue(LLoot);
}

Loot::~Loot() {}

LootType Loot::category() const {
    return m_category;
}

bool Loot::move(Atlas& atlas, const QList<MyItem*>& obstacles) {
    Q_UNUSED(obstacles);
    if (m_stateCnt >= m_criticalCnt) {
        QRectF rect;
        QList<QPoint> points = premove(rect, m_dir);
        if (movable(points, atlas)) {
            setPos(rect.center());
        } else {
            turnBack();
        }
    }
    return true;
}
