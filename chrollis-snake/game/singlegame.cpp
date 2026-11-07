#include "singlegame.h"

SingleGame::SingleGame(QGraphicsScene* scene) : Game(scene) {
    m_snake = new Snake(scene);
    m_occupied.insert(m_snake->init({gridSize / 4, gridSize / 2}, Right));
}

SingleGame::~SingleGame() {}

void SingleGame::update() {
    if (!m_snake->is_empty()) {
        QPoint new_head_pos = m_snake->next_head_position();
        if (Snake::is_border(new_head_pos)) {
            over = 1;
            kill(m_snake);
        } else if (m_occupied.contains(new_head_pos)) {
            auto entity = m_occupied.find(new_head_pos);
            switch ((*entity)->get_type()) {
                case Head:
                case Body:
                case Tail:
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
        m_snake->update();
    }
}

void SingleGame::keyPressEvent(QKeyEvent* event) {
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
}

void SingleGame::mousePressEvent(QMouseEvent* event) {}
