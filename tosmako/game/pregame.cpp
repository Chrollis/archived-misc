#include "pregame.h"

PreGame::PreGame(QGraphicsScene* scene) : Game(scene) {
    m_snake = new Snake(scene);
    m_occupied.insert(m_snake->init({gridSize / 4, gridSize / 2}, Right));
    m_target = m_snake->tail()->position();
    rect = new QGraphicsRectItem(0, 0, snakeSize, snakeSize);
    rect->setPen(QPen(QColor(0x00FF00)));
    m_scene->addItem(rect);
    create_tomato();
    set_target();
}

PreGame::~PreGame() {
    m_scene->removeItem(rect);
    delete rect;
}

QList<Direction> PreGame::find_path(const QPoint& tomato_pos) {
    QList<Direction> direction_list;
    QPoint target = tomato_pos;
    QMap<QPoint, Entity*> v_occupied = m_occupied;
    v_occupied.remove(m_snake->tail()->position());

    const QPoint directions[4] = {{0, -1}, {-1, 0}, {0, 1}, {1, 0}};
    QMultiMap<int, Node*> open_list;
    QSet<Node*> close_list;

    Node* current = new Node(m_snake->head()->position(), target);
    Node* next;
    close_list.insert(current);

    bool flag = 1;
    int find_path_counter = 0;
    do {
        for (const auto& dir : directions) {
            next = new Node(current, dir, target);
            if (Snake::is_walkable(next->pos, v_occupied) && std::find_if(close_list.begin(), close_list.end(), [&](Node* obj) { return obj->pos == next->pos; }) == close_list.end()) {
                if (!open_list.contains(next->F(), next)) {
                    if (next->H == 0) {
                        flag = 0;
                        break;
                    }
                    open_list.insert(next->F(), next);
                } else {
                    auto temp = open_list.find(next->F(), next);
                    if (next->G < (*temp)->G) {
                        delete *temp;
                        *temp = next;
                    }
                }
            }
        }
        if (open_list.empty()) {
            return {};
        }
        current = *open_list.begin();
        open_list.remove(current->F(), current);
        close_list.insert(current);
    } while (flag && find_path_counter++ < 512);

    for (; next->parent != nullptr; next = next->parent) {
        QPoint drc = next->pos - next->parent->pos;
        auto result = std::find(directions, directions + 4, drc);
        if (result == directions + 4) {
        } else {
            Direction dir = Direction(result - directions);
            direction_list.prepend(dir);
        }
    }

    for (auto& node : open_list) {
        delete node;
    }

    for (auto& node : close_list) {
        delete node;
    }
    return direction_list;
}

int Node::penalty(Snake* snake) {
    int wall = 0;
    int density = 0;
    int narrow = 0;

    if (snake != nullptr) {
        const int safe_wall = 1;

        int dist[4] = {pos.y(), pos.x(), gridSize - pos.y(), gridSize - pos.x()};
        auto calc_wall = [&](Direction dir) -> int { return (dist[dir] < safe_wall) ? (safe_wall - dist[dir]) * 10 : 0; };
        for (int i = 0; i < 4; i++) {
            wall += calc_wall((Direction)i);
        }

        const int scanR = 2;
        for (int dx = -scanR; dx <= scanR; dx++) {
            for (int dy = -scanR; dy <= scanR; dy++) {
                if (dx == 0 && dy == 0) {
                    continue;
                }
                QPoint next(pos.x() + dx, pos.y() + dy);
                if (Snake::is_border(next)) {
                    density += 15;
                    continue;
                }
                for (int i = 0; i < snake->bodies().size() - 1; i++) {
                    if (next == snake->bodies()[i]->position()) {
                        int dist = abs(dx) + abs(dy);
                        density += 20 / (dist + 1);
                        break;
                    }
                }
            }
        }

        auto check_dir = [&](int dx, int dy) -> int {
            int free = 0;

            QPoint check = pos;
            while (1) {
                check += QPoint(dx, dy);
                if (Snake::is_border(check)) {
                    break;
                }
                bool occupied = 0;
                for (const auto& body : snake->bodies()) {
                    if (body->position() == check) {
                        occupied = 1;
                        break;
                    }
                }
                if (occupied) {
                    break;
                }
                free++;
            }
            check = pos;
            while (1) {
                check -= QPoint(dx, dy);
                if (Snake::is_border(check)) {
                    break;
                }
                bool occupied = 0;
                for (const auto& body : snake->bodies()) {
                    if (body->position() == check) {
                        occupied = 1;
                        break;
                    }
                }
                if (occupied) {
                    break;
                }
                free++;
            }
            return free;
        };

        int horizon = check_dir(1, 0);
        int vertical = check_dir(0, 1);
        if (horizon < 3) {
            narrow += (3 - horizon) * 30;
        }
        if (vertical < 3) {
            narrow += (3 - vertical) * 30;
        }
    }
    return wall + density + narrow;
}

void PreGame::set_target() {
    int index = 0;
    auto tar_it = m_occupied.begin();
    auto dar_id = m_occupied.begin();
    for (auto it = m_occupied.begin(); it != m_occupied.end(); it++, index++) {
        if ((*it)->get_type() == Domado) {
            if (((*it)->position() - m_snake->head()->position()).manhattanLength() < ((*dar_id)->position() - m_snake->head()->position()).manhattanLength()) {
                dar_id = m_occupied.begin();
                std::advance(dar_id, index);
            }
        } else if ((*it)->get_type() == Tomato) {
            if (((*it)->position() - m_snake->head()->position()).manhattanLength() > ((*tar_it)->position() - m_snake->head()->position()).manhattanLength()) {
                tar_it = m_occupied.begin();
                std::advance(tar_it, index);
            }
        }
    }
    if ((*dar_id)->get_type() != Stone && (*tar_it)->get_type() != Stone) {
        if ((*dar_id)->get_type() != Domado) {
            m_target = (*tar_it)->position();
        } else {
            m_target = (*dar_id)->position();
        }
    }
    rect->setRect(m_target.x() * snakeSize, m_target.y() * snakeSize, snakeSize, snakeSize);
}

void PreGame::update() {
    if (retarget_counter++ > 256) {
        set_target();
        retarget_counter = 0;
    }
    if (!m_snake->is_empty()) {
        if (!direction_list.empty()) {
            m_snake->set_next(direction_list.front());
            direction_list.pop_front();
        } else {
            direction_list = find_path(m_target);
            if (!direction_list.empty()) {
                m_snake->set_next(direction_list.front());
                direction_list.pop_front();
            } else {
                direction_list = find_path(m_snake->tail()->position());
                if (!direction_list.empty()) {
                    m_snake->set_next(direction_list.front());
                    direction_list.pop_front();
                }
            }
        }
        if (m_snake->if_turn(m_occupied)) {
            m_snake->set_next(Direction((m_snake->head()->next() + 1) % 4));
            if (m_snake->if_turn(m_occupied)) {
                m_snake->set_next(Direction((m_snake->head()->next() + 2) % 4));
                if (m_snake->if_turn(m_occupied)) {
                    kill(m_snake);
                    over = 1;
                }
            }
        }
        if (!m_snake->is_empty()) {
            QPoint new_head_pos = m_snake->next_head_position();
            if (m_occupied.contains(new_head_pos)) {
                auto entity = m_occupied.find(new_head_pos);
                switch ((*entity)->get_type()) {
                    case Head:
                    case Body:
                    case Tail:
                    case Stone:
                        kill(m_snake);
                        over = 1;
                        break;
                    case Domado:
                        m_score += 40;
                    case Tomato:
                        m_score += 10;
                        m_scene->removeItem(*entity);
                        delete *entity;

                        m_occupied.remove(new_head_pos);
                        move(1);
                        if (m_score % 50 == 0) {
                            up = 1;
                        }
                        break;
                    case Clean:
                        break;
                }
            } else {
                move(0);
            }
            if (new_head_pos == m_target) {
                set_target();
            }
        }
        m_snake->update();
    }
}
void PreGame::keyPressEvent(QKeyEvent* event) {}

void PreGame::mousePressEvent(QMouseEvent* event) {
    if (event->buttons() == Qt::LeftButton) {
        QPoint pos;
        pos.rx() = (event->pos().x() - gridEdge) / snakeSize;
        pos.ry() = (event->pos().y() - gridEdge - 25) / snakeSize;
        if (!Snake::is_border(pos) && Snake::is_walkable(pos, m_occupied)) {
            m_target = pos;
            rect->setRect(m_target.x() * snakeSize, m_target.y() * snakeSize, snakeSize, snakeSize);
            direction_list = find_path(m_target);
        }
    }
}

Node::Node(QPoint pos, QPoint tar, Snake* snake) {
    this->parent = nullptr;
    this->pos = pos;
    this->G = 0;
    this->H = (pos - tar).manhattanLength() * 20 + penalty(snake);
}

Node::Node(Node* parent, QPoint dir, QPoint tar, Snake* snake) {
    this->parent = parent;
    this->pos = parent->pos + dir;
    this->G = parent->G + 10;
    this->H = (this->pos - tar).manhattanLength() * 20 + penalty(snake);
}

int Node::F() {
    return G + H;
}

bool operator==(const Node& a, const Node& b) {
    return a.pos == b.pos;
}
