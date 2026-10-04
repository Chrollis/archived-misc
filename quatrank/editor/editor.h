#ifndef EDITOR_H
#define EDITOR_H

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMainWindow>
#include <QMap>
#include "utils.h"

class QAction;
class QActionGroup;
class QComboBox;
class QLabel;
class QListWidget;
class QSlider;

struct LevelData {
    QVector<BlockType> blocks = QVector<BlockType>(1024, Ground);
    QRect home = QRect(360, 336, 48, 48);
    QRect spawn = QRect(360, 24, 48, 48);
};

class EditorView : public QGraphicsView {
    Q_OBJECT
public:
    explicit EditorView(QGraphicsScene* scene, QWidget* parent = nullptr);

signals:
    void cellClicked(QPoint cell);
    void cellDragged(QPoint cell);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    QPoint cellAt(const QPoint& pos) const;
};

class Editor : public QMainWindow {
    Q_OBJECT
public:
    explicit Editor(QWidget* parent = nullptr);

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveFileAs();
    void undo();
    void redo();
    void paintCell(const QPoint& cell);
    void addLevel();
    void swapLevels();
    void selectLevel(int row);
    void generateRandomMap();

private:
    struct Snapshot {
        int level;
        QVector<BlockType> blocks;
        QRect home;
        QRect spawn;
    };

    void setupUi();
    void renderMap();
    void loadFile(const QString& path);
    void saveTo(const QString& path);
    void pushSnapshot();
    void applySnapshot(const Snapshot& s);
    void updateLevelList();
    void updateActions();
    void setStatus(const QString& text);

    QMap<int, LevelData> m_levels;
    int m_currentLevel = 1;
    BlockType m_brush = Brick;
    bool m_placingHome = false;
    bool m_placingSpawn = false;
    int m_brushSize = 1;
    QString m_filePath;
    QList<Snapshot> m_undoStack;
    QList<Snapshot> m_redoStack;

    QGraphicsScene* m_scene;
    EditorView* m_view;
    QGraphicsPixmapItem* m_mapItem;
    QListWidget* m_levelList;
    QComboBox* m_swapA;
    QComboBox* m_swapB;
    QSlider* m_brushSizeSlider;
    QLabel* m_brushSizeLabel;
    QActionGroup* m_brushGroup;
    QAction* m_homeAction;
    QAction* m_spawnAction;
    QAction* m_undoAction;
    QAction* m_redoAction;
    QLabel* m_statusLabel;
};

#endif