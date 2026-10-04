#include "pvpgame.h"

PVPGame::PVPGame(QGraphicsScene* scene) : CoworkGame(scene) {}

PVPGame::~PVPGame() {}

int PVPGame::score() {
    return qMax(m_score, n_score);
}

void PVPGame::update() {
    if (!m_snake->is_empty()) {
        QPoint new_head_pos = m_snake->next_head_position();
        if (Snake::is_border(new_head_pos)) {
            over = 1;
            kill(m_snake);
            m_score = n_score;
        } else if (m_occupied.contains(new_head_pos)) {
            auto entity = m_occupied.find(new_head_pos);
            switch ((*entity)->get_type()) {
                case Head:
                    if (n_snake->contains(*entity)) {
                        if (m_score < n_score) {
                            m_score = n_score;
                            kill(m_snake);
                        } else {
                            n_score = m_score;
                            kill(n_snake);
                        }
                    }
                    over = 1;
                    break;
                case Body:
                case Tail:
                case Stone:
                    over = 1;
                    kill(m_snake);
                    m_score = n_score;
                    break;
                case Domado:
                    m_score += 40;
                case Tomato:
                    m_score += 10;
                    m_scene->removeItem(*entity);
                    delete *entity;

                    m_occupied.remove(new_head_pos);
                    move(1);
                    if (score() % 50 == 0) {
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
    if (!n_snake->is_empty()) {
        QPoint new_head_pos = n_snake->next_head_position();
        if (Snake::is_border(new_head_pos)) {
            over = 1;
            kill(n_snake);
            n_score = m_score;
        } else if (m_occupied.contains(new_head_pos)) {
            auto entity = m_occupied.find(new_head_pos);
            switch ((*entity)->get_type()) {
                case Head:
                case Body:
                case Tail:
                case Stone:
                    over = 1;
                    kill(n_snake);
                    n_score = m_score;
                    break;
                case Domado:
                    n_score += 40;
                case Tomato:
                    n_score += 10;
                    m_scene->removeItem(*entity);
                    delete *entity;

                    m_occupied.remove(new_head_pos);
                    n_move(1);
                    if (score() % 50 == 0) {
                        up = 1;
                    }
                    break;
                case Clean:
                    break;
            }
        } else {
            n_move(0);
        }
        n_snake->update();
    }
    text_move();
}
