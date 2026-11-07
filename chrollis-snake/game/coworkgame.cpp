#include "coworkgame.h"

#include <QApplication>

CoworkGame::CoworkGame(QGraphicsScene* scene) : Game(scene) {
    m_snake = new Snake(scene);
    m_occupied.insert(m_snake->init({gridSize / 4, gridSize / 4}, Right));
    n_snake = new Snake(scene);
    m_occupied.insert(n_snake->init({gridSize / 4, gridSize / 4 * 3}, Right));

    text_1p = new QGraphicsPixmapItem(QPixmap(":/png/assets/objects/1P.png"));
    text_1p->setZValue(3);
    m_scene->addItem(text_1p);
    text_2p = new QGraphicsPixmapItem(QPixmap(":/png/assets/objects/2P.png"));
    text_2p->setZValue(3);
    m_scene->addItem(text_2p);

    text_move();
}

CoworkGame::~CoworkGame() {
    kill(n_snake);
    delete n_snake;
    m_scene->removeItem(text_1p);
    delete text_1p;
    m_scene->removeItem(text_2p);
    delete text_2p;
}

void CoworkGame::update() {
    if (m_snake->is_empty() && n_snake->is_empty()) {
        over = 1;
        kill(m_snake);
        kill(n_snake);
        return;
    }
    if (!m_snake->is_empty()) {
        QPoint new_head_pos = m_snake->next_head_position();
        if (Snake::is_border(new_head_pos)) {
            kill(m_snake);
        } else if (m_occupied.contains(new_head_pos)) {
            auto entity = m_occupied.find(new_head_pos);
            switch ((*entity)->get_type()) {
                case Head:
                case Body:
                case Tail:
                    if (m_snake->contains(*entity)) {
                        kill(m_snake);
                    } else {
                        if (m_snake->if_turn(m_occupied)) {
                            m_snake->set_next(Direction((m_snake->head()->next() + 1) % 4));
                            if (m_snake->if_turn(m_occupied)) {
                                m_snake->set_next(Direction((m_snake->head()->next() + 2) % 4));
                                if (m_snake->if_turn(m_occupied)) {
                                    kill(m_snake);
                                }
                            }
                        }
                        if (!m_snake->is_empty()) {
                            move(0);
                        }
                    }
                    break;
                case Stone:
                    over = 1;
                    kill(m_snake);
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
    }
    if (!n_snake->is_empty()) {
        QPoint new_head_pos = n_snake->next_head_position();
        if (Snake::is_border(new_head_pos)) {
            kill(n_snake);
        } else if (m_occupied.contains(new_head_pos)) {
            auto entity = m_occupied.find(new_head_pos);
            switch ((*entity)->get_type()) {
                case Head:
                case Body:
                case Tail:
                    if (n_snake->contains(*entity)) {
                        kill(n_snake);
                    } else {
                        if (n_snake->if_turn(m_occupied)) {
                            n_snake->set_next(Direction((n_snake->head()->next() + 1) % 4));
                            if (n_snake->if_turn(m_occupied)) {
                                n_snake->set_next(Direction((n_snake->head()->next() + 2) % 4));
                                if (n_snake->if_turn(m_occupied)) {
                                    kill(n_snake);
                                }
                            }
                        }
                        if (!n_snake->is_empty()) {
                            n_move(0);
                        }
                    }
                    break;
                case Stone:
                    over = 1;
                    kill(n_snake);
                    break;
                case Domado:
                    m_score += 40;
                case Tomato:
                    m_score += 10;
                    m_scene->removeItem(*entity);

                    delete *entity;
                    m_occupied.remove(new_head_pos);
                    n_move(1);
                    if (m_score % 50 == 0) {
                        up = 1;
                    }
                    break;
                case Clean:
                    break;
            }
        } else {
            n_move(0);
        }
    }
    m_snake->update();
    n_snake->update();
    text_move();
}

void CoworkGame::keyPressEvent(QKeyEvent* event) {
    if (!m_snake->is_empty()) {
        switch (event->key()) {
            case Qt::Key::Key_W:
                if (m_snake->head()->diretion() != Down) {
                    m_snake->head()->set_next(Up);
                }
                break;
            case Qt::Key::Key_A:
                if (m_snake->head()->diretion() != Right) {
                    m_snake->head()->set_next(Left);
                }
                break;
            case Qt::Key::Key_S:
                if (m_snake->head()->diretion() != Up) {
                    m_snake->head()->set_next(Down);
                }
                break;
            case Qt::Key::Key_D:
                if (m_snake->head()->diretion() != Left) {
                    m_snake->head()->set_next(Right);
                }
                break;
        }
    }
    if (!n_snake->is_empty()) {
        switch (event->key()) {
            case Qt::Key::Key_I:
                if (n_snake->head()->diretion() != Down) {
                    n_snake->head()->set_next(Up);
                }
                break;
            case Qt::Key::Key_J:
                if (n_snake->head()->diretion() != Right) {
                    n_snake->head()->set_next(Left);
                }
                break;
            case Qt::Key::Key_K:
                if (n_snake->head()->diretion() != Up) {
                    n_snake->head()->set_next(Down);
                }
                break;
            case Qt::Key::Key_L:
                if (n_snake->head()->diretion() != Left) {
                    n_snake->head()->set_next(Right);
                }
                break;
        }
    }
}

void CoworkGame::mousePressEvent(QMouseEvent* event) {}

void CoworkGame::n_move(bool grow) {
    n_snake->push_head();
    m_occupied.insert(n_snake->head()->position(), n_snake->head());
    if (!grow) {
        m_occupied.remove(n_snake->pop_tail());
    }
}

void CoworkGame::text_move() {
    if (m_snake->is_empty()) {
        text_1p->setVisible(0);
    } else {
        text_1p->setPos(m_snake->head()->pos());
    }
    if (n_snake->is_empty()) {
        text_2p->setVisible(0);
    } else {
        text_2p->setPos(n_snake->head()->pos());
    }
}
