#ifndef ORIGAME_H
#define ORIGAME_H

#include <QKeyEvent>
#include <QSoundEffect>
#include "hostile.h"
#include "player.h"

class OriGame {
private:
    QList<QSoundEffect*> m_sounds;
    QList<Bullet*> m_bullets;
    QList<Loot*> m_loots;
    QList<Hostile*> m_hostiles;
    Atlas* m_atlas;
    Player* m_player;
    QGraphicsScene* m_scene;

private:
    int m_level;
    int m_restHostileAmt;
    bool m_baseDestroyed;
    bool m_paused;
    int m_cycle;
    int m_targetCnt;
    qreal m_volume;
    bool m_cheatIndestructible;
    bool m_cheatInfiniteAmmo;
    bool m_cheatNoCooldown;
    bool m_cheatBaseShield;

public:
    OriGame(QGraphicsScene* scene);
    ~OriGame();
    void clear();
    void init(int level);
    void summonPlayer();
    void summonHostile();
    void refresh();
    int update();
    int level() const;
    int restHostileAmt() const;
    int playerBackup() const;
    int playerRank() const;
    bool playerBoosting() const;
    int playerBoostRemaining() const;
    int playerBoostCooldown() const;
    bool isPaused() const;
    void setPaused(bool paused);
    qreal volume() const;
    void setVolume(qreal volume);
    void setCheatIndestructible(bool on);
    void setCheatInfiniteAmmo(bool on);
    void setCheatNoCooldown(bool on);
    void setCheatBaseShield(bool on);

public:
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void addSound(QString path);
};

#endif  // ORIGAME_H
