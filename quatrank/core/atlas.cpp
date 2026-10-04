#include "atlas.h"

void Atlas::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            int type = m_blocks[j * 32 + i];

            if (type == Forest) {
                painter->drawImage(i * 24, j * 24, imgBlock[Ground]);
            }
            painter->drawImage(i * 24, j * 24, imgBlock[type]);
        }
    }
    painter->drawImage(m_rectSpawn, imgSpawn[CycleIndex[m_spawnCnt / FramesPerPicture]]);
    m_spawnCnt = (++m_spawnCnt) % m_criticalCnt;

    if (m_stateCnt > m_criticalCnt) {
        if ((m_stateCnt / (FramesPerPicture / 2)) % 2) {
            painter->drawImage(m_rectHome, whiteSlashTexture(imgHome[0]));
        } else {
            painter->drawImage(m_rectHome, imgHome[0]);
        }
        m_stateCnt -= 1;
    } else if (m_stateCnt == m_criticalCnt) {
        painter->drawImage(m_rectHome, imgHome[0]);
    } else if (m_stateCnt > 0) {
        if ((m_stateCnt / (FramesPerPicture / 2)) % 2) {
            painter->drawImage(m_rectHome, whiteSlashTexture(imgHome[1 + (m_criticalCnt - m_stateCnt) / FramesPerPicture]));
        } else {
            painter->drawImage(m_rectHome, whiteSlashTexture(imgHome[1 + (m_criticalCnt - m_stateCnt) / FramesPerPicture]));
        }
        m_stateCnt -= 1;
    } else {
        painter->drawImage(m_rectHome, imgHome[4]);
    }
}

Atlas::Atlas(int level, QGraphicsScene* scene)
    : MyItem(QSize(768, 768), QPoint(0, 0), Up, 0, 4 * FramesPerPicture, scene), m_level(level), m_spawnCnt(0), m_imgFrontForest(768, 768, QImage::Format_ARGB32), m_forestLayer(nullptr) {
    read(level);
    setZValue(LAtlas);

    m_forestLayer = new QGraphicsPixmapItem(this);
    m_forestLayer->setZValue(LForest);
    rebuildForestLayer();
}

Atlas::~Atlas() {}

void Atlas::rebuildForestLayer() {
    m_imgFrontForest.fill(Qt::transparent);
    QPainter forestPainter(&m_imgFrontForest);
    forestPainter.setOpacity(0.5);
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            if (m_blocks[j * 32 + i] == Forest) {
                forestPainter.drawImage(i * 24, j * 24, imgBlock[Forest]);
            }
        }
    }
    forestPainter.end();
    m_forestLayer->setPixmap(QPixmap::fromImage(m_imgFrontForest));
}

void Atlas::save(int level) {
    QSettings set(getResourcePath("data/map-data.ini"), QSettings::IniFormat);
    QString pre = QString("Level-%1").arg(level);
    QByteArray bytes;
    unsigned char counter = 1;
    unsigned char type = m_blocks[0];
    for (int i = 1; i < 1024; i++) {
        if (m_blocks[i] == type && counter < 31) {
            counter += 1;
        } else {
            bytes.append((type << 5) + counter);
            counter = 1;
            type = m_blocks[i];
        }
    }
    bytes.append((type << 5) + counter);
    set.setValue(pre + "/blocks", bytes.toBase64());
    set.setValue(pre + "/home", m_rectHome);
    set.setValue(pre + "/spawn", m_rectSpawn);
}

int Atlas::totalLevels() {
    QSettings set(getResourcePath("data/map-data.ini"), QSettings::IniFormat);
    int maxLevel = 0;
    for (const QString& group : set.childGroups()) {
        if (!group.startsWith("Level-")) {
            continue;
        }
        bool ok = false;
        int n = group.mid(6).toInt(&ok);
        if (ok && n > maxLevel) {
            maxLevel = n;
        }
    }
    return qMax(maxLevel, 1);
}

void Atlas::read(int level) {
    QSettings set(getResourcePath("data/map-data.ini"), QSettings::IniFormat);
    QString pre = QString("Level-%1").arg(level);
    const QByteArray bytes = QByteArray::fromBase64(set.value(pre + "/blocks").toByteArray());
    m_blocks = QVector<BlockType>(1024, Ground);
    auto it = m_blocks.begin();
    for (const unsigned char& byte : bytes) {
        if (it == m_blocks.end()) {
            break;
        }
        unsigned char type = (byte >> 5) % 6;
        for (unsigned char num = byte & 31; it != m_blocks.end() && num != 0; num--) {
            *(it++) = (BlockType)type;
        }
    }

    m_rectHome = set.value(pre + "/home", QRect(0, 0, 44, 44)).toRect();
    m_rectSpawn = set.value(pre + "/spawn", QRect(0, 0, 44, 44)).toRect();
    m_level = level;
    m_stateCnt = 4 * FramesPerPicture + 5 * FramesPerSecond;
    save(level);

    if (m_forestLayer != nullptr) {
        rebuildForestLayer();
    }
}

BlockType Atlas::block(QPoint pos) const {
    return m_blocks[pos.y() * 32 + pos.x()];
}

const QRect& Atlas::rectHome() const {
    return m_rectHome;
}

const QRect& Atlas::rectSpawn() const {
    return m_rectSpawn;
}

const QImage& Atlas::imgFrontForest() const {
    return m_imgFrontForest;
}

void Atlas::setBlock(QPoint pos, BlockType type) {
    BlockType old = m_blocks[pos.y() * 32 + pos.x()];
    m_blocks[pos.y() * 32 + pos.x()] = type;

    if (old == Forest || type == Forest) {
        rebuildForestLayer();
    }
}

void Atlas::setRectHome(QPoint center) {
    m_rectHome.moveCenter(center);
}

void Atlas::setRectSpawn(QPoint center) {
    m_rectSpawn.moveCenter(center);
}
