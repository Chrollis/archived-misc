#include "player.h"

void Player::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    QPixmap pic;
    QTransform trans;
    trans.rotate(-90 * m_dir);
    if (m_stateCnt > m_criticalCnt) {
        trans.scale((double)m_criticalCnt / m_stateCnt, (double)m_criticalCnt / m_stateCnt);
        if ((m_stateCnt / (FramesPerPicture / 2)) % 2) {
            pic = QPixmap::fromImage(whiteSlashTexture(imgPlayer[m_rank - 1])).transformed(trans);
        } else {
            pic = QPixmap::fromImage(imgPlayer[m_rank - 1]).transformed(trans);
        }
        m_stateCnt -= 1;
        painter->drawPixmap(-pic.width() / 2, -pic.width() / 2, pic);
    } else if (m_stateCnt == m_criticalCnt) {
        pic = QPixmap::fromImage(imgPlayer[m_rank - 1]).transformed(trans);
        painter->drawPixmap(-22, -22, pic);
    } else if (m_stateCnt > 0) {
        pic = QPixmap::fromImage(imgBang[m_stateCnt / FramesPerPicture]);
        m_stateCnt -= 1;
        painter->drawPixmap(-48, -48, pic);
    } else {
        disconnect();
    }
    checkBullets();
    m_shootCnt += 1;
}

Player::Player(QPoint pos, Direction dir, int rank, QGraphicsScene* scene) : OriTank(pos, dir, 3, rank, scene), m_backup(3), m_boostCnt(0), m_boostCooldown(0), m_noCooldown(false) {}

Player::~Player() {}

bool Player::hit() {
    if (m_stateCnt == m_criticalCnt) {
        m_heart -= 1;
        if (m_heart <= 0) {
            m_backup -= 1;
            bomb();
            return true;
        }
        m_stateCnt = m_criticalCnt + FramesPerSecond;
    }
    return false;
}

bool Player::move(Atlas& atlas, const QList<MyItem*>& obstacles) {
    if (m_boostCnt > 0) {
        m_boostCnt -= 1;
        if (m_boostCnt == 0) {
            m_speed = 3;
            m_boostCooldown = m_noCooldown ? 0 : 5 * FramesPerSecond;
        }
    } else if (m_boostCooldown > 0) {
        m_boostCooldown -= 1;
    }
    if (m_engine && m_stateCnt >= m_criticalCnt) {
        QRectF rect;
        QList<QPoint> points = premove(rect, m_dir);
        if (movable(points, atlas) && !blockedBy(rect, obstacles) && !rect.intersects(QRectF(atlas.rectHome()))) {
            setPos(rect.center());
        } else {
            return false;
        }
    }
    return true;
}

void Player::fetchLoot(Loot& loot) {
    switch (loot.category()) {
        case Pistol:
            m_bulletSpeed = qMin(m_bulletSpeed + 1, 12);
            break;
        case Shell:
            m_maxBulletAmt = qMin(m_maxBulletAmt + 1, 10);
            break;
        case Star:
            m_rank = qMin(m_rank + 1, 3);
            m_heart = qMin(m_heart + 1, 10);
            m_maxBulletAmt = qMin(m_maxBulletAmt + 1, 10);
            break;
        case Backup:
            m_backup = qMin(m_backup + 1, 20);
            break;
    }
    loot.disconnect();
}

void Player::recover(QPoint pos, Direction dir, int rank, QGraphicsScene* scene) {
    setPos(pos);
    m_dir = dir;
    m_rank = rank;
    m_heart = rank;
    m_bulletSpeed = 6;
    m_bullets.clear();
    m_maxBulletAmt = rank;
    m_shootCnt = 0;
    m_size = {44, 44};
    reconnect(scene);
    m_stateCnt = m_criticalCnt + FramesPerSecond;
    m_engine = false;
    m_backup -= 1;
    m_boostCnt = 0;
    m_boostCooldown = 0;
    m_speed = 3;
}

int Player::backup() const {
    return m_backup;
}

int Player::rank() const {
    return m_rank;
}

void Player::setBoost(bool on) {
    if (on && m_boostCnt == 0 && (m_boostCooldown == 0 || m_noCooldown)) {
        m_boostCnt = 3 * FramesPerSecond;
        m_speed = 6;
    }
}

void Player::setNoCooldown(bool on) {
    m_noCooldown = on;
    if (on) {
        m_boostCooldown = 0;
    }
}

bool Player::isBoosting() const {
    return m_boostCnt > 0;
}

int Player::boostRemaining() const {
    return m_boostCnt;
}

int Player::boostCooldown() const {
    return m_boostCooldown;
}
