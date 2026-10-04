#include "oritank.h"

OriTank::OriTank(QPoint pos, Direction dir, int speed, int rank, QGraphicsScene* scene)
    : Entity(QSize(44, 44), pos, dir, speed, 5 * FramesPerPicture, scene),
      m_rank(rank),
      m_heart(rank),
      m_shootCnt(0),
      m_bulletSpeed(2 * speed),
      m_maxBulletAmt(rank),
      m_engine(false),
      m_infiniteAmmo(false) {
    m_stateCnt = m_criticalCnt + FramesPerSecond;
    setZValue(LTank);
}

OriTank::~OriTank() {}

bool OriTank::hit() {
    if (m_stateCnt == m_criticalCnt) {
        m_heart -= 1;
        if (m_heart <= 0) {
            bomb();
            return true;
        }
        m_stateCnt = m_criticalCnt + FramesPerSecond;
    }
    return false;
}

bool OriTank::fire(QList<Bullet*>& allBullets) {
    if (m_stateCnt == m_criticalCnt && m_shootCnt >= 2 * FramesPerPicture && (m_infiniteAmmo || m_bullets.size() < m_maxBulletAmt)) {
        int x = -1, y = -1;
        switch (m_dir) {
            case Up:
                x = this->x();
                y = this->y() - 28;
                break;
            case Left:
                x = this->x() - 28;
                y = this->y();
                break;
            case Down:
                x = this->x();
                y = this->y() + 28;
                break;
            case Right:
                x = this->x() + 28;
                y = this->y();
                break;
        }
        Bullet* temp = new Bullet(QPoint(x, y), m_dir, m_bulletSpeed, this, m_scene);
        m_bullets.prepend(temp);
        allBullets.prepend(temp);
        m_shootCnt = 0;
        return true;
    }
    return false;
}

bool OriTank::bomb() {
    if (m_stateCnt == m_criticalCnt) {
        m_size = {96, 96};
        m_stateCnt -= 1;
        for (Bullet* bullet : m_bullets) {
            bullet->disconnect();
        }
        m_bullets.clear();
        return true;
    }
    return false;
}

void OriTank::checkBullets() {
    for (auto it = m_bullets.begin(); it != m_bullets.end();) {
        if (!(*it)->isConnected()) {
            it = m_bullets.erase(it);
        } else {
            it++;
        }
    }
}

void OriTank::setEngine(bool engine) {
    m_engine = engine;
}

void OriTank::setDirection(Direction dir) {
    m_dir = dir;
}

void OriTank::setInfiniteAmmo(bool on) {
    m_infiniteAmmo = on;
}
