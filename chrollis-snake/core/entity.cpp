#include "entity.h"

Entity::Entity(EntityType type, QGraphicsItem* parent) : QGraphicsItem(parent), m_type(type) {}

Entity::Entity(const Entity& other) {
    m_type = other.m_type;
    m_direction = other.m_direction;
    n_direction = other.n_direction;
    set_position(other.m_position);
}

void Entity::set_position(QPoint pos) {
    m_position = pos;
    setPos(pos.x() * snakeSize + snakeSize / 2, pos.y() * snakeSize + snakeSize / 2);
}

QPoint Entity::position() const {
    return m_position;
}

void Entity::set_direction(Direction this_dir) {
    m_direction = this_dir;
}

void Entity::set_next(Direction next_dir) {
    n_direction = next_dir;
}

void Entity::set_type(EntityType type) {
    m_type = type;
}

EntityType Entity::get_type() const {
    return m_type;
}

void Entity::add_life_counter() {
    life_counter++;
}

Direction Entity::diretion() const {
    return m_direction;
}

Direction Entity::next() const {
    return n_direction;
}

QRectF Entity::boundingRect() const {
    if (m_type == Tomato || m_type == Domado) {
        return QRectF(-tomatoSize / 2, -tomatoSize / 2, tomatoSize, tomatoSize);
    } else {
        return QRectF(-snakeSize / 2, -snakeSize / 2, snakeSize, snakeSize);
    }
}

void Entity::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);
    QPixmap t_pic;
    QTransform transform;
    if (m_type == Tomato) {
        t_pic.load(":/png/assets/objects/tomato.png");
    } else if (m_type == Domado) {
        QImage t_img(":/png/assets/objects/domado.png");
        if (life_counter % 8 >= 4) {
            for (int y = 0; y < t_img.height(); ++y) {
                for (int x = 0; x < t_img.width(); ++x) {
                    QRgb pixel = t_img.pixel(x, y);
                    t_img.setPixel(x, y, qRgba(255 - qGray(pixel), 255 - qGray(pixel), 255 - qGray(pixel), qAlpha(pixel)));
                }
            }
        }
        if (life_counter >= 48) {
            m_type = Clean;
        }
        t_pic = QPixmap::fromImage(t_img);
    } else if (m_type == Head) {
        t_pic.load(":/png/assets/objects/head.png");
        transform.rotate(-90 * m_direction);
        t_pic = t_pic.transformed(transform);
    } else if (m_type == Body) {
        if (m_direction == n_direction) {
            t_pic.load(":/png/assets/objects/body.png");
            transform.rotate(-90 * m_direction);
            t_pic = t_pic.transformed(transform);
        } else {
            t_pic.load(":/png/assets/objects/bodyTurn.png");
            if ((n_direction - m_direction + 4) % 4 == 1) {
                transform.scale(-1, 1);
                t_pic = t_pic.transformed(transform);
            }
            transform.reset();
            transform.rotate(-90 * m_direction);
            t_pic = t_pic.transformed(transform);
        }
    } else if (m_type == Tail) {
        t_pic.load(":/png/assets/objects/tinyTail.png");
        transform.rotate(-90 * n_direction);
        t_pic = t_pic.transformed(transform);
    } else if (m_type == Stone) {
        t_pic.load(":/png/assets/objects/stone.png");
    }
    painter->drawPixmap(boundingRect().x(), boundingRect().y(), t_pic);
}

bool operator<(const QPoint& a, const QPoint& b) {
    return a.x() < b.x() || (a.x() == b.x() && a.y() < b.y());
}
