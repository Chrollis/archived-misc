#include "origame.h"

OriGame::OriGame(QGraphicsScene* scene) {
    m_scene = scene;
    m_level = 0;
    m_restHostileAmt = 0;
    m_baseDestroyed = false;
    m_paused = false;
    m_cycle = 0;
    m_targetCnt = 0;
    m_volume = 1.0;
    m_cheatIndestructible = false;
    m_cheatInfiniteAmmo = false;
    m_cheatNoCooldown = false;
    m_cheatBaseShield = false;
    m_atlas = nullptr;
    m_player = nullptr;
}

OriGame::~OriGame() {
    clear();
}

void OriGame::clear() {
    for (auto it = m_sounds.begin(); it != m_sounds.end();) {
        delete *it;
        it = m_sounds.erase(it);
    }
    for (auto it = m_bullets.begin(); it != m_bullets.end();) {
        (*it)->disconnect();
        delete *it;
        it = m_bullets.erase(it);
    }
    for (auto it = m_loots.begin(); it != m_loots.end();) {
        (*it)->disconnect();
        delete *it;
        it = m_loots.erase(it);
    }
    for (auto it = m_hostiles.begin(); it != m_hostiles.end();) {
        (*it)->disconnect();
        delete *it;
        it = m_hostiles.erase(it);
    }
    if (m_atlas != nullptr) {
        m_atlas->disconnect();
        delete m_atlas;
        m_atlas = nullptr;
    }
    if (m_player != nullptr) {
        m_player->disconnect();
        delete m_player;
        m_player = nullptr;
    }
}

void OriGame::init(int level) {
    clear();

    if (level > Atlas::totalLevels()) {
        level = 1;
        m_cycle += 1;
    }
    m_level = level;
    m_baseDestroyed = false;
    m_restHostileAmt = 20 + (level - 1) * 5 + m_cycle * 10;
    m_atlas = new Atlas(level, m_scene);
    m_player = new Player(m_atlas->rectHome().center() + QPoint(0, 72), Up, 1, m_scene);

    m_player->setInfiniteAmmo(m_cheatInfiniteAmmo);
    m_player->setNoCooldown(m_cheatNoCooldown);
    addSound(getResourcePath("audio/start.wav"));
}

void OriGame::summonPlayer() {
    if (!m_player->isConnected() && m_player->backup() > 0) {
        m_player->recover(m_atlas->rectHome().center() + QPoint(0, 72), Up, 1, m_scene);
        addSound(getResourcePath("audio/recover.wav"));
    }
}

void OriGame::summonHostile() {
    for (auto it = m_hostiles.begin(); it != m_hostiles.end(); it++) {
        if ((*it)->rect().intersects(m_atlas->rectSpawn())) {
            return;
        }
    }
    if (m_restHostileAmt > 0 && m_hostiles.size() < 4) {
        int rank = qMin(rand() % 4 + 1 + m_cycle, 4);
        Hostile* temp = new Hostile(m_atlas->rectSpawn().center(), (Direction)(rand() % 4), rank, m_scene);
        m_hostiles.prepend(temp);
    }
}

void OriGame::refresh() {
    bool flag[4] = {false, false, false, false};

    m_targetCnt += 1;
    if (m_targetCnt >= 3 * FramesPerSecond) {
        m_targetCnt = 0;
        for (auto it = m_hostiles.begin(); it != m_hostiles.end(); ++it) {
            if (!m_player->isConnected()) {
                (*it)->setWandering(true);
                continue;
            }

            if (rand() % 100 < (*it)->wanderChance()) {
                (*it)->setWandering(true);
                continue;
            }
            (*it)->setWandering(false);
            if (rand() % 100 < 60) {
                (*it)->setTarget(m_player->rect().center());
            } else {
                (*it)->setTarget(QRectF(m_atlas->rectHome()).center());
            }
        }
    }
    for (auto it = m_sounds.begin(); it != m_sounds.end();) {
        if (!(*it)->isPlaying()) {
            delete *it;
            it = m_sounds.erase(it);
        } else {
            ++it;
        }
    }
    for (auto it = m_bullets.begin(); it != m_bullets.end();) {
        if (!(*it)->isConnected()) {
            delete *it;
            it = m_bullets.erase(it);
        } else {
            ++it;
        }
    }
    for (auto it = m_hostiles.begin(); it != m_hostiles.end();) {
        if (!(*it)->isConnected()) {
            delete *it;
            it = m_hostiles.erase(it);
        } else {
            ++it;
        }
    }
    for (auto it = m_loots.begin(); it != m_loots.end();) {
        if (!(*it)->isConnected()) {
            delete *it;
            it = m_loots.erase(it);
        } else if ((*it)->rect().intersects(m_player->rect())) {
            m_player->fetchLoot(**it);
            delete *it;
            it = m_loots.erase(it);
        } else {
            (*it)->move(*m_atlas);
            ++it;
        }
    }
    for (auto it = m_bullets.begin(); it != m_bullets.end(); ++it) {
        if ((*it)->isInteractable()) {
            if ((*it)->rect().contains(m_atlas->rectHome().center())) {
                (*it)->bomb();
                flag[2] = true;

                if ((*it)->maker() != m_player && !m_cheatBaseShield && m_atlas->bomb()) {
                    flag[0] = true;
                    m_baseDestroyed = true;
                }
            } else if ((*it)->rect().intersects(m_player->rect())) {
                (*it)->bomb();
                flag[2] = true;
                if ((*it)->maker() != m_player && !m_cheatIndestructible) {
                    if (m_player->hit()) {
                        flag[1] = true;
                    }
                }
            } else {
                bool temp = true;
                for (auto at = m_hostiles.begin(); at != m_hostiles.end(); ++at) {
                    if ((*it)->rect().intersects((*at)->rect())) {
                        (*it)->bomb();
                        flag[2] = true;
                        temp = false;
                        if ((*it)->maker() != *at) {
                            if ((*at)->hit()) {
                                m_restHostileAmt -= 1;
                                flag[1] = true;
                                if ((*at)->isFlickered()) {
                                    Loot* temp = new Loot(QPoint((*at)->x(), (*at)->y()), (*at)->dir(), 8, (LootType)(rand() % 4), m_scene);
                                    m_loots.prepend(temp);
                                }
                            }
                        }
                    }
                }
                if (temp) {
                    for (auto at = it + 1; at != m_bullets.end(); ++at) {
                        if ((*at)->isInteractable()) {
                            if ((*it)->rect().intersects((*at)->rect())) {
                                (*it)->bomb();
                                (*at)->bomb();
                                flag[2] = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    for (auto it = m_bullets.begin(); it != m_bullets.end(); ++it) {
        if (!(*it)->move(*m_atlas)) {
            flag[2] = true;
        }
    }
    QList<MyItem*> obstacles;
    for (auto it = m_hostiles.begin(); it != m_hostiles.end(); ++it) {
        obstacles.append(*it);
    }
    m_player->move(*m_atlas, obstacles);
    for (auto it = m_hostiles.begin(); it != m_hostiles.end(); ++it) {
        QList<MyItem*> hostileObstacles;
        hostileObstacles.append(m_player);
        for (auto at = m_hostiles.begin(); at != m_hostiles.end(); ++at) {
            if (at != it) {
                hostileObstacles.append(*at);
            }
        }
        (*it)->move(*m_atlas, hostileObstacles);
        bool temp = true;
        QRectF rect = (*it)->rect();
        QPointF pos;
        switch ((*it)->dir()) {
            case Up:
                pos = rect.bottomLeft();
                rect.setHeight(768);
                rect.setBottomLeft(pos);
                break;
            case Left:
                pos = rect.bottomRight();
                rect.setWidth(768);
                rect.setBottomRight(pos);
                break;
            case Down:
                pos = rect.topRight();
                rect.setHeight(768);
                rect.setTopRight(pos);
                break;
            case Right:
                pos = rect.topLeft();
                rect.setWidth(768);
                rect.setTopLeft(pos);
                break;
            default:
                break;
        }
        for (auto at = m_hostiles.begin(); at != m_hostiles.end(); ++at) {
            if (at != it && (*at)->rect().intersects(rect)) {
                temp = false;
            }
        }
        if (temp && (m_player->rect().intersects(rect) || QRectF(m_atlas->rectHome()).intersects(rect))) {
            if ((*it)->fire(m_bullets)) {
                flag[3] = true;
            }
        }
    }
    if (flag[0]) {
        addSound(getResourcePath("audio/beep.wav"));
    }
    if (flag[1]) {
        addSound(getResourcePath("audio/bang.wav"));
    }
    if (flag[2]) {
        addSound(getResourcePath("audio/hit.wav"));
    }
    if (flag[3]) {
        addSound(getResourcePath("audio/fire.wav"));
    }
}

int OriGame::update() {
    if (m_paused) {
        m_scene->update();
        return 0;
    }
    refresh();
    summonHostile();
    m_scene->update();
    if (m_baseDestroyed || (m_player->backup() <= 0 && !m_player->isConnected())) {
        return -1;
    } else if (m_restHostileAmt <= 0 && m_hostiles.isEmpty()) {
        return 1;
    } else {
        return 0;
    }
}

int OriGame::level() const {
    return m_level;
}

int OriGame::restHostileAmt() const {
    return m_restHostileAmt;
}

int OriGame::playerBackup() const {
    return m_player != nullptr ? m_player->backup() : 0;
}

int OriGame::playerRank() const {
    return m_player != nullptr ? m_player->rank() : 0;
}

bool OriGame::playerBoosting() const {
    return m_player != nullptr && m_player->isBoosting();
}

int OriGame::playerBoostRemaining() const {
    return m_player != nullptr ? m_player->boostRemaining() : 0;
}

int OriGame::playerBoostCooldown() const {
    return m_player != nullptr ? m_player->boostCooldown() : 0;
}

bool OriGame::isPaused() const {
    return m_paused;
}

void OriGame::setPaused(bool paused) {
    m_paused = paused;
}

qreal OriGame::volume() const {
    return m_volume;
}

void OriGame::setVolume(qreal volume) {
    m_volume = qBound<qreal>(0.0, volume, 1.0);
}

void OriGame::setCheatIndestructible(bool on) {
    m_cheatIndestructible = on;
}

void OriGame::setCheatInfiniteAmmo(bool on) {
    m_cheatInfiniteAmmo = on;
    if (m_player != nullptr) {
        m_player->setInfiniteAmmo(on);
    }
}

void OriGame::setCheatNoCooldown(bool on) {
    m_cheatNoCooldown = on;
    if (m_player != nullptr) {
        m_player->setNoCooldown(on);
    }
}

void OriGame::setCheatBaseShield(bool on) {
    m_cheatBaseShield = on;
}

void OriGame::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key::Key_Escape) {
        exit(0);
        return;
    }
    if (event->key() == Qt::Key::Key_P) {
        m_paused = !m_paused;
        return;
    }
    if (m_player != nullptr) {
        if (event->key() == Qt::Key::Key_Return) {
            m_player->setBoost(true);
        } else if (event->key() == Qt::Key::Key_Shift) {
            m_player->setEngine(true);
        } else if (event->key() == Qt::Key::Key_X) {
            m_player->turnBack();
        } else if (event->key() == Qt::Key::Key_Q) {
            m_player->turnLeft();
        } else if (event->key() == Qt::Key::Key_E) {
            m_player->turnRight();
        } else if (event->key() == Qt::Key::Key_Space) {
            if (m_player->fire(m_bullets)) {
                addSound(getResourcePath("audio/fire.wav"));
            }
        } else if (event->key() == Qt::Key::Key_R) {
            summonPlayer();
        } else if (event->key() == Qt::Key::Key_W) {
            m_player->setDir(Up);
        } else if (event->key() == Qt::Key::Key_A) {
            m_player->setDir(Left);
        } else if (event->key() == Qt::Key::Key_S) {
            m_player->setDir(Down);
        } else if (event->key() == Qt::Key::Key_D) {
            m_player->setDir(Right);
        }
    }
}

void OriGame::keyReleaseEvent(QKeyEvent* event) {
    if (m_player != nullptr) {
        if (event->key() == Qt::Key::Key_Shift) {
            m_player->setEngine(false);
        }
    }
}

void OriGame::addSound(QString path) {
    QSoundEffect* sound = new QSoundEffect;
    sound->setSource(QUrl::fromLocalFile(path));
    sound->setLoopCount(1);
    sound->setVolume(m_volume);
    sound->play();
    m_sounds.prepend(sound);
}
