#include "myitem.h"

QList<QPoint> MyItem::premove(QRectF& rect, Direction dir) {
    QList<QPoint> points;
    rect = this->rect();
    switch (dir) {
        case Up:
            rect.moveTop(rect.top() - m_speed);
            for (int i = (int)rect.left() / 24; i <= (int)rect.right() / 24; i++) {
                points.push_back(QPoint(i, ((int)rect.top() >= 0 ? (int)rect.top() : (int)rect.top() - 23) / 24));
            }
            break;
        case Left:
            rect.moveLeft(rect.left() - m_speed);
            for (int i = (int)rect.top() / 24; i <= (int)rect.bottom() / 24; i++) {
                points.push_back(QPoint(((int)rect.left() >= 0 ? (int)rect.left() : (int)rect.left() - 23) / 24, i));
            }
            break;
        case Down:
            rect.moveBottom(rect.bottom() + m_speed);
            for (int i = (int)rect.left() / 24; i <= (int)rect.right() / 24; i++) {
                points.push_back(QPoint(i, (int)rect.bottom() / 24));
            }
            break;
        case Right:
            rect.moveRight(rect.right() + m_speed);
            for (int i = (int)rect.top() / 24; i <= (int)rect.bottom() / 24; i++) {
                points.push_back(QPoint((int)rect.right() / 24, i));
            }
            break;
    }
    return points;
}

MyItem::MyItem(QSize size, QPoint pos, Direction dir, int speed, int criticalCnt, QGraphicsScene* scene)
    : m_size(size), m_dir(dir), m_speed(speed), m_stateCnt(criticalCnt), m_criticalCnt(criticalCnt) {
    setPos(pos);
    if ((m_scene = scene) != nullptr) {
        m_scene->addItem(this);
    }
}

MyItem::~MyItem() {
    disconnect();
}

void MyItem::reconnect(QGraphicsScene* scene) {
    disconnect();
    if (scene != nullptr) {
        m_scene = scene;
        m_scene->addItem(this);
    }
}

void MyItem::disconnect() {
    if (m_scene != nullptr) {
        m_scene->removeItem(this);
        m_scene = nullptr;
    }
}

bool MyItem::bomb() {
    if (m_stateCnt == m_criticalCnt) {
        m_stateCnt -= 1;
        return true;
    }
    return false;
}

void MyItem::setDir(Direction dir) {
    m_dir = dir;
}

QRectF MyItem::boundingRect() const {
    return QRectF(-m_size.width() * 0.5, -m_size.height() * 0.5, m_size.width(), m_size.height());
}

QRectF MyItem::rect() const {
    QRectF rect(-m_size.width() * 0.5, -m_size.height() * 0.5, m_size.width(), m_size.height());
    rect.moveCenter(pos());
    return rect;
}

Direction MyItem::dir() const {
    return m_dir;
}

bool MyItem::isConnected() const {
    return m_scene != nullptr;
}

bool MyItem::isInteractable() const {
    return m_stateCnt == m_criticalCnt;
}
