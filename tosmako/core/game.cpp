#include "game.h"

Game::Game(QGraphicsScene* scene) {
    m_scene = scene;
}

Game::~Game() {
    kill(m_snake);
    delete m_snake;
    for (auto& entity : m_occupied) {
        m_scene->removeItem(entity);
        delete entity;
    }
    m_occupied.clear();
    m_scene = nullptr;
    // qDebug() << "Game: delete m_snake; each entity in m_occupied";
}

int Game::score() {
    return m_score;
}

void Game::create_tomato() {
    QPoint tomato_pos;
    do {
        tomato_pos.setX(QRandomGenerator::global()->bounded(gridSize));
        tomato_pos.setY(QRandomGenerator::global()->bounded(gridSize));
    } while (m_occupied.contains(tomato_pos));
    int rate = QRandomGenerator::global()->bounded(100);
    Entity* tomato = new Entity(rate <= 70 ? Tomato : (rate <= 90 ? Stone : Domado));
    tomato->set_position(tomato_pos);
    m_scene->addItem(tomato);
    tomato->setZValue(1);
    m_occupied.insert(tomato_pos, tomato);
    // qDebug() << "Game:: new tomato";
}

void Game::move(bool grow) {
    m_snake->push_head();
    m_occupied.insert(m_snake->head()->position(), m_snake->head());
    if (!grow) {
        m_occupied.remove(m_snake->pop_tail());
    }
}

void Game::kill(Snake* snake) {
    QList list = snake->clear();
    for (auto& pos : list) {
        m_occupied.remove(pos);
    }
}

bool Game::is_over() {
    return over;
}

bool Game::is_up() {
    if (up) {
        up = 0;
        return 1;
    } else {
        return 0;
    }
}

void Game::update_tomato() {
    QList<Entity*> occupied = m_occupied.values();
    QList<QPoint> clean_list;
    for (const auto& entity : std::as_const(occupied)) {
        // // qDebug() << "Game:" << entity->position() << entity->get_type();
        if (entity->get_type() == Domado) {
            entity->add_life_counter();
            entity->update();
        } else if (entity->get_type() == Clean) {
            Entity* clean = entity;
            m_scene->removeItem(clean);
            clean_list.append(clean->position());
            delete clean;
            // qDebug() << "Game: delete clean-type entity";
        }
    }
    // // qDebug() << "Game: for over";
    for (auto& pos : clean_list) {
        m_occupied.remove(pos);
    }
}
