#include "hostile.h"

#include <queue>

static QList<QPoint> findPath(const Atlas& atlas, QPoint start, QPoint goal, const QList<MyItem*>& obstacles) {
    auto idx = [](int x, int y) { return y * 32 + x; };
    auto passable = [&](int x, int y) {
        if (x < 0 || x >= 32 || y < 0 || y >= 32) {
            return false;
        }
        BlockType t = atlas.block(QPoint(x, y));
        if (t == Brick || t == Steel || t == River) {
            return false;
        }

        if (x != goal.x() || y != goal.y()) {
            for (const MyItem* obstacle : obstacles) {
                if (obstacle->isConnected()) {
                    QRectF r = obstacle->rect();
                    QPoint oc((int)(r.center().x() / 24), (int)(r.center().y() / 24));
                    if (oc.x() == x && oc.y() == y) {
                        return false;
                    }
                }
            }
        }
        return true;
    };
    if (!passable(goal.x(), goal.y())) {
        return {};
    }

    QVector<int> g(1024, -1);
    QVector<int> parent(1024, -1);

    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> open;
    auto h = [](int x, int y, int gx, int gy) { return qAbs(x - gx) + qAbs(y - gy); };
    int startIdx = idx(start.x(), start.y());
    int goalIdx = idx(goal.x(), goal.y());
    g[startIdx] = 0;
    open.push({h(start.x(), start.y(), goal.x(), goal.y()), startIdx});
    const int dx[4] = {0, -1, 0, 1};
    const int dy[4] = {-1, 0, 1, 0};
    while (!open.empty()) {
        auto top = open.top();
        open.pop();
        int cur = top.second;
        if (cur == goalIdx) {
            break;
        }
        int cx = cur % 32, cy = cur / 32;
        for (int d = 0; d < 4; d++) {
            int nx = cx + dx[d], ny = cy + dy[d];
            if (!passable(nx, ny)) {
                continue;
            }
            int ni = idx(nx, ny);
            int ng = g[cur] + 1;
            if (g[ni] == -1 || ng < g[ni]) {
                g[ni] = ng;
                parent[ni] = cur;
                open.push({ng + h(nx, ny, goal.x(), goal.y()), ni});
            }
        }
    }
    if (g[goalIdx] == -1) {
        return {};
    }

    QList<QPoint> path;
    int cur = goalIdx;
    while (cur != startIdx) {
        path.prepend(QPoint(cur % 32, cur / 32));
        cur = parent[cur];
    }
    return path;
}

void Hostile::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    QPixmap pic;
    QTransform trans;
    trans.rotate(-90 * m_dir);
    if (m_stateCnt > m_criticalCnt) {
        trans.scale((double)m_criticalCnt / m_stateCnt, (double)m_criticalCnt / m_stateCnt);
        if ((m_stateCnt / (FramesPerPicture / 2)) % 2) {
            if (m_flickered) {
                pic = QPixmap::fromImage(whiteSlashTexture(imgHostile[m_rank - 1]) + imgFlicker[CycleIndex[m_flickerCnt / FramesPerPicture]]).transformed(trans);
            } else {
                pic = QPixmap::fromImage(whiteSlashTexture(imgHostile[m_rank - 1])).transformed(trans);
            }
        } else {
            if (m_flickered) {
                pic = QPixmap::fromImage(imgHostile[m_rank - 1] + imgFlicker[CycleIndex[m_flickerCnt / FramesPerPicture]]).transformed(trans);
            } else {
                pic = QPixmap::fromImage(imgHostile[m_rank - 1]).transformed(trans);
            }
        }
        m_stateCnt -= 1;
        painter->drawPixmap(-pic.width() / 2, -pic.width() / 2, pic);
    } else if (m_stateCnt == m_criticalCnt) {
        if (m_flickered) {
            pic = QPixmap::fromImage(imgHostile[m_rank - 1] + imgFlicker[CycleIndex[m_flickerCnt / FramesPerPicture]]).transformed(trans);
        } else {
            pic = QPixmap::fromImage(imgHostile[m_rank - 1]).transformed(trans);
        }
        painter->drawPixmap(-22, -22, pic);
    } else if (m_stateCnt > 0) {
        pic = QPixmap::fromImage(imgBang[m_stateCnt / FramesPerPicture]);
        m_stateCnt -= 1;
        painter->drawPixmap(-48, -48, pic);
    } else {
        disconnect();
    }
    m_flickerCnt = (++m_flickerCnt) % (4 * FramesPerPicture);
    checkBullets();
    m_shootCnt += rand() % 2;
    m_engineCnt += rand() % 2;
}

Hostile::Hostile(QPoint pos, Direction dir, int rank, QGraphicsScene* scene)
    : OriTank(pos, dir, 2, rank, scene),
      m_flickered(rand() % 100 > 60),
      m_flickerCnt(0),
      m_engineCnt(0),
      m_target(pos),
      m_pathCnt(0),
      m_wanderChance(qMax(10, 50 - rank * 10)),
      m_wanderCnt(0),
      m_wanderDir(Up) {
    m_engine = true;
}

Hostile::~Hostile() {}

bool Hostile::isFlickered() const {
    return m_flickered;
}

void Hostile::setTarget(const QPointF& pos) {
    m_target = pos;
}

void Hostile::setWandering(bool on) {
    if (on) {
        m_wanderCnt = FramesPerSecond * (5 - m_rank) / 2 + rand() % FramesPerSecond;
        m_wanderDir = (Direction)(rand() % 4);
    } else {
        m_wanderCnt = 0;
    }
}

int Hostile::wanderChance() const {
    return m_wanderChance;
}

void Hostile::wander(Atlas& atlas, const QList<MyItem*>& obstacles) {
    m_wanderCnt -= 1;
    QRectF rect;
    QList<QPoint> points = premove(rect, m_wanderDir);
    if (movable(points, atlas) && !blockedBy(rect, obstacles) && !rect.intersects(QRectF(atlas.rectHome()))) {
        m_dir = m_wanderDir;
        setPos(rect.center());
    } else {
        m_wanderDir = (Direction)(rand() % 4);
    }
}

bool Hostile::move(Atlas& atlas, const QList<MyItem*>& obstacles) {
    if (!m_engine && m_engineCnt >= 2 * FramesPerSecond) {
        m_engine = true;
        m_engineCnt = 0;
    }
    if (m_engine && m_engineCnt >= 5 * FramesPerSecond) {
        m_engine = false;
        m_engineCnt = 0;
    }
    if (m_engine && m_stateCnt >= m_criticalCnt) {
        if (m_wanderCnt > 0) {
            wander(atlas, obstacles);
            return true;
        }

        m_pathCnt += 1;
        if (m_pathCnt >= FramesPerSecond || m_path.isEmpty()) {
            m_pathCnt = 0;
            QPoint start((int)(x() / 24), (int)(y() / 24));
            QPoint goal((int)(m_target.x() / 24), (int)(m_target.y() / 24));
            m_path = findPath(atlas, start, goal, obstacles);

            if (m_path.isEmpty()) {
                setWandering(true);
                wander(atlas, obstacles);
                return true;
            }
        }
        if (!m_path.isEmpty()) {
            QPointF pos = this->pos();
            QPointF target(m_path.first().x() * 24 + 12, m_path.first().y() * 24 + 12);

            if (qAbs(pos.x() - target.x()) < 12 && qAbs(pos.y() - target.y()) < 12) {
                m_path.removeFirst();
                if (!m_path.isEmpty()) {
                    target = QPointF(m_path.first().x() * 24 + 12, m_path.first().y() * 24 + 12);
                }
            }
            if (!m_path.isEmpty()) {
                Direction dir = m_dir;
                if (qAbs(target.x() - pos.x()) > qAbs(target.y() - pos.y())) {
                    dir = target.x() > pos.x() ? Right : Left;
                } else {
                    dir = target.y() > pos.y() ? Down : Up;
                }
                if (dir != m_dir) {
                    m_dir = dir;
                    return true;
                }
                QRectF rect;
                QList<QPoint> points = premove(rect, m_dir);
                if (movable(points, atlas) && !blockedBy(rect, obstacles) && !rect.intersects(QRectF(atlas.rectHome()))) {
                    setPos(rect.center());
                } else {
                    m_path.clear();
                    m_pathCnt = FramesPerSecond;
                    return false;
                }
                return true;
            }
        }

        setWandering(true);
        wander(atlas, obstacles);
    }
    return true;
}
