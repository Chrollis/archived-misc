#include "snake.h"

Snake::Snake(QGraphicsScene* scene) {
    m_scene = scene;
}

bool Snake::is_border(const QPoint& pos) {
    return pos.x() < 0 || pos.x() >= gridSize || pos.y() < 0 || pos.y() >= gridSize;
}

bool Snake::is_walkable(const QPoint& pos, const QMap<QPoint, Entity*>& occupied) {
    if (is_border(pos)) {
        return 0;
    } else if (!occupied.contains(pos)) {
        return 1;
    } else {
        Entity* entity = occupied.value(pos);
        switch (entity->get_type()) {
            case Head:
            case Body:
            case Tail:
            case Stone:
                return 0;
            case Clean:
            case Tomato:
            case Domado:
                return 1;
        }
    }
    return 0;
}

QPoint Snake::next_head_position() {
    QPoint new_head_pos = m_bodies.head()->position();
    const QPoint directions[4] = {{0, -1}, {-1, 0}, {0, 1}, {1, 0}};
    new_head_pos += directions[m_bodies.head()->next()];
    return new_head_pos;
}

void Snake::push_head() {
    Entity* old_head = m_bodies.head();
    Entity* new_head = new Entity(*old_head);

    new_head->set_direction(old_head->next());
    QPoint new_head_pos = next_head_position();
    new_head->set_position(new_head_pos);
    m_bodies.prepend(new_head);
    m_scene->addItem(new_head);
    new_head->setZValue(2);
    old_head->set_type(Body);
}

QPoint Snake::pop_tail() {
    Entity* old_tail = m_bodies.back();
    m_scene->removeItem(old_tail);
    QPoint old_tail_pos = old_tail->position();
    delete old_tail;
    m_bodies.pop_back();
    m_bodies.back()->set_type(Tail);

    return old_tail_pos;
}

QMap<QPoint, Entity*> Snake::init(const QPoint& start, Direction dir) {
    QMap<QPoint, Entity*> occupied;
    const QPoint directions[4] = {{0, -1}, {-1, 0}, {0, 1}, {1, 0}};

    Entity* head = new Entity(Head);

    head->set_position(start);
    head->set_direction(dir);
    head->set_next(dir);
    m_bodies.append(head);
    m_scene->addItem(head);
    head->setZValue(2);
    occupied.insert(start, head);

    Entity* body = new Entity(Body);

    body->set_position(start - directions[dir]);
    body->set_direction(dir);
    body->set_next(dir);
    m_bodies.append(body);
    m_scene->addItem(body);
    body->setZValue(2);
    occupied.insert(start - directions[dir], body);

    Entity* tail = new Entity(Tail);

    tail->set_position(start - directions[dir] * 2);
    tail->set_direction(dir);
    tail->set_next(dir);
    m_bodies.append(tail);
    m_scene->addItem(tail);
    tail->setZValue(2);
    occupied.insert(start - directions[dir] * 2, tail);

    return occupied;
}

QList<QPoint> Snake::clear() {
    QList<QPoint> occupied;
    for (auto& body : m_bodies) {
        m_scene->removeItem(body);
        occupied.append(body->position());
        delete body;
    }
    m_bodies.clear();

    return occupied;
}

void Snake::update() {
    for (const auto& body : std::as_const(m_bodies)) {
        body->update();
    }
}

bool Snake::if_turn(const QMap<QPoint, Entity*> occupied) {
    QPoint new_head_pos = next_head_position();
    if (Snake::is_border(new_head_pos)) {
        return 1;
    } else {
        auto entity = occupied.find(new_head_pos);
        if (entity == occupied.end()) {
            return 0;
        } else if ((*entity)->get_type() == Tomato || (*entity)->get_type() == Domado) {
            return 0;
        } else {
            return 1;
        }
    }
}

bool Snake::contains(Entity* tar) {
    return m_bodies.contains(tar);
}

void Snake::set_next(Direction next) {
    m_bodies.head()->set_next(next);
}

bool Snake::is_empty() {
    return m_bodies.empty();
}

Entity* Snake::head() {
    return m_bodies.head();
}

Entity* Snake::tail() {
    return m_bodies.back();
}

const QQueue<Entity*>& Snake::bodies() {
    return m_bodies;
}
