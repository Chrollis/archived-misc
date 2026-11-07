#ifndef ATLAS_H
#define ATLAS_H

#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QSettings>
#include "myitem.h"

class Atlas : public MyItem {
private:
    QVector<BlockType> m_blocks;
    int m_level;
    int m_spawnCnt;
    QImage m_imgFrontForest;
    QGraphicsPixmapItem* m_forestLayer;
    QRect m_rectHome;
    QRect m_rectSpawn;

private:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;
    void rebuildForestLayer();

public:
    Atlas(int level, QGraphicsScene* scene = nullptr);
    ~Atlas();
    void save(int level);
    void read(int level);
    static int totalLevels();

public:
    BlockType block(QPoint pos) const;
    const QRect& rectHome() const;
    const QRect& rectSpawn() const;
    const QImage& imgFrontForest() const;

public:
    void setBlock(QPoint pos, BlockType type);
    void setRectHome(QPoint center);
    void setRectSpawn(QPoint center);
};

#endif  // ATLAS_H
