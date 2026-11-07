#include "bullet.h"

void Bullet::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    QPixmap pic;
    QTransform trans;
    trans.rotate(-90 * m_dir);
    if (m_stateCnt >= m_criticalCnt) {
        pic = QPixmap::fromImage(imgBullet[0]).transformed(trans);
        painter->drawPixmap(-6, -6, pic);
    } else if (m_stateCnt > 0) {
        pic = QPixmap::fromImage(imgBullet[1]).transformed(trans);
        painter->drawPixmap(-6, -6, pic);
        m_stateCnt -= 1;
    } else {
        disconnect();
    }
}

Bullet::Bullet(QPoint pos, Direction dir, int speed, MyItem* maker, QGraphicsScene* scene) : Entity(QSize(12, 12), pos, dir, speed, FramesPerPicture, scene), m_maker(maker) {
    setZValue(LBullet);
}

Bullet::~Bullet() {
    if (m_maker != nullptr) {
        m_maker = nullptr;
    }
}

const MyItem* const Bullet::maker() const {
    return m_maker;
}

bool Bullet::move(Atlas& atlas, const QList<MyItem*>& obstacles) {
    Q_UNUSED(obstacles);
    if (m_stateCnt >= m_criticalCnt) {
        QRectF rect;
        QList<QPoint> points = premove(rect, m_dir);
        if (movable(points, atlas)) {
            setPos(rect.center());
        } else {
            if (bomb()) {
                return false;
            }
        }
    }
    return true;
}

bool Bullet::movable(const QList<QPoint>& points, Atlas& atlas) {
    bool flag = true;
    for (const QPoint& point : points) {
        if (!QRect(0, 0, 32, 32).contains(point)) {
            flag = false;
        } else {
            switch (atlas.block(point)) {
                case Brick:
                    atlas.setBlock(point, Ground);
                case Steel:
                    flag = false;
                    break;
                default:
                    break;
            }
        }
    }
    return flag;
}
