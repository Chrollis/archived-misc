#include "editor.h"

#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSettings>
#include <QSlider>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>

static QByteArray encodeBlocks(const QVector<BlockType>& blocks) {
    QByteArray bytes;
    unsigned char counter = 1;
    unsigned char type = blocks[0];
    for (int i = 1; i < 1024; i++) {
        if (blocks[i] == type && counter < 31) {
            counter += 1;
        } else {
            bytes.append((type << 5) + counter);
            counter = 1;
            type = blocks[i];
        }
    }
    bytes.append((type << 5) + counter);
    return bytes;
}

static QVector<BlockType> decodeBlocks(const QByteArray& bytes) {
    QVector<BlockType> blocks(1024, Ground);
    auto it = blocks.begin();
    for (const unsigned char& byte : bytes) {
        if (it == blocks.end()) {
            break;
        }
        unsigned char type = (byte >> 5) % 6;
        for (unsigned char num = byte & 31; it != blocks.end() && num != 0; num--) {
            *(it++) = (BlockType)type;
        }
    }
    return blocks;
}

EditorView::EditorView(QGraphicsScene* scene, QWidget* parent) : QGraphicsView(scene, parent) {
    setRenderHint(QPainter::SmoothPixmapTransform, false);
}

QPoint EditorView::cellAt(const QPoint& pos) const {
    QPointF p = mapToScene(pos);
    return QPoint((int)(p.x() / 24), (int)(p.y() / 24));
}

void EditorView::mousePressEvent(QMouseEvent* event) {
    QGraphicsView::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
        emit cellClicked(cellAt(event->pos()));
    }
}

void EditorView::mouseMoveEvent(QMouseEvent* event) {
    QGraphicsView::mouseMoveEvent(event);
    if (event->buttons() & Qt::LeftButton) {
        emit cellDragged(cellAt(event->pos()));
    }
}

Editor::Editor(QWidget* parent) : QMainWindow(parent) {
    loadResourceImages();
    setupUi();
    newFile();
    updateActions();
}

void Editor::setupUi() {
    setWindowTitle("Quatrank Map Editor");

    QMenu* fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction("New", QKeySequence::New, this, &Editor::newFile);
    fileMenu->addAction("Open...", QKeySequence::Open, this, &Editor::openFile);
    fileMenu->addAction("Save", QKeySequence::Save, this, &Editor::saveFile);
    fileMenu->addAction("Save As...", QKeySequence::SaveAs, this, &Editor::saveFileAs);
    fileMenu->addSeparator();
    fileMenu->addAction("Random Map...", this, &Editor::generateRandomMap);
    QMenu* editMenu = menuBar()->addMenu("Edit");
    m_undoAction = editMenu->addAction("Undo", QKeySequence::Undo, this, &Editor::undo);
    m_redoAction = editMenu->addAction("Redo", QKeySequence::Redo, this, &Editor::redo);

    QToolBar* toolBar = addToolBar("Tools");
    toolBar->setMovable(false);
    m_brushGroup = new QActionGroup(this);
    m_brushGroup->setExclusive(true);
    for (int i = 0; i < 6; i++) {
        QAction* act = toolBar->addAction(QIcon(QPixmap::fromImage(imgBlock[i])), QString());
        act->setCheckable(true);
        act->setData(i);
        m_brushGroup->addAction(act);
        if (i == Brick) {
            act->setChecked(true);
        }
    }
    toolBar->addSeparator();

    m_homeAction = toolBar->addAction(QIcon(QPixmap::fromImage(imgHome[0])), "Home");
    m_homeAction->setCheckable(true);
    m_brushGroup->addAction(m_homeAction);
    m_spawnAction = toolBar->addAction(QIcon(QPixmap::fromImage(imgSpawn[0])), "Spawn");
    m_spawnAction->setCheckable(true);
    m_brushGroup->addAction(m_spawnAction);
    toolBar->addSeparator();

    toolBar->addWidget(new QLabel(" Brush: "));
    m_brushSizeSlider = new QSlider(Qt::Horizontal);
    m_brushSizeSlider->setRange(1, 5);
    m_brushSizeSlider->setValue(1);
    m_brushSizeSlider->setFixedWidth(80);
    toolBar->addWidget(m_brushSizeSlider);
    m_brushSizeLabel = new QLabel("1");
    toolBar->addWidget(m_brushSizeLabel);
    toolBar->addSeparator();
    toolBar->addAction("Save", this, &Editor::saveFile);

    QWidget* central = new QWidget;
    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    QWidget* panel = new QWidget;
    panel->setFixedWidth(180);
    QVBoxLayout* panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(0, 0, 0, 0);
    panelLayout->addWidget(new QLabel("Levels:"));
    m_levelList = new QListWidget;
    panelLayout->addWidget(m_levelList);
    QPushButton* newLevelBtn = new QPushButton("New Level");
    panelLayout->addWidget(newLevelBtn);
    QHBoxLayout* swapLayout = new QHBoxLayout;
    m_swapA = new QComboBox;
    m_swapB = new QComboBox;
    QPushButton* swapBtn = new QPushButton("Swap");
    swapLayout->addWidget(m_swapA);
    swapLayout->addWidget(m_swapB);
    swapLayout->addWidget(swapBtn);
    panelLayout->addLayout(swapLayout);
    panelLayout->addStretch();
    mainLayout->addWidget(panel);

    m_scene = new QGraphicsScene(this);
    m_view = new EditorView(m_scene, this);
    m_view->setSceneRect(0, 0, 768, 768);
    m_mapItem = m_scene->addPixmap(QPixmap());
    mainLayout->addWidget(m_view);
    setCentralWidget(central);

    m_statusLabel = new QLabel;
    statusBar()->addWidget(m_statusLabel);

    connect(m_levelList, &QListWidget::currentRowChanged, this, &Editor::selectLevel);
    connect(newLevelBtn, &QPushButton::clicked, this, &Editor::addLevel);
    connect(swapBtn, &QPushButton::clicked, this, &Editor::swapLevels);
    connect(m_brushSizeSlider, &QSlider::valueChanged, this, [this](int v) {
        m_brushSize = v;
        m_brushSizeLabel->setText(QString::number(v));
    });
    connect(m_brushGroup, &QActionGroup::triggered, this, [this](QAction* act) {
        if (act == m_homeAction) {
            m_placingHome = true;
            m_placingSpawn = false;
        } else if (act == m_spawnAction) {
            m_placingSpawn = true;
            m_placingHome = false;
        } else {
            m_placingHome = false;
            m_placingSpawn = false;
            m_brush = (BlockType)act->data().toInt();
        }
    });
    connect(m_view, &EditorView::cellClicked, this, &Editor::paintCell);
    connect(m_view, &EditorView::cellDragged, this, &Editor::paintCell);
}

void Editor::newFile() {
    m_levels.clear();
    m_undoStack.clear();
    m_redoStack.clear();
    m_currentLevel = 1;
    m_levels[1] = LevelData();
    m_filePath.clear();
    updateLevelList();
    renderMap();
    setStatus("New file");
    updateActions();
}

void Editor::openFile() {
    QString path = QFileDialog::getOpenFileName(this, "Open Map Data", QString(), "INI files (*.ini)");
    if (path.isEmpty()) {
        return;
    }
    loadFile(path);
}

void Editor::loadFile(const QString& path) {
    QSettings set(path, QSettings::IniFormat);
    m_levels.clear();
    for (const QString& group : set.childGroups()) {
        if (!group.startsWith("Level-")) {
            continue;
        }
        bool ok = false;
        int n = group.mid(6).toInt(&ok);
        if (!ok || n <= 0) {
            continue;
        }
        LevelData data;
        data.blocks = decodeBlocks(QByteArray::fromBase64(set.value(group + "/blocks").toByteArray()));
        data.home = set.value(group + "/home", QRect(360, 336, 48, 48)).toRect();
        data.spawn = set.value(group + "/spawn", QRect(360, 24, 48, 48)).toRect();
        m_levels[n] = data;
    }
    if (m_levels.isEmpty()) {
        m_levels[1] = LevelData();
    }
    m_currentLevel = m_levels.firstKey();
    m_undoStack.clear();
    m_redoStack.clear();
    m_filePath = path;
    updateLevelList();
    renderMap();
    setStatus("Loaded " + path);
    updateActions();
}

void Editor::saveFile() {
    if (m_filePath.isEmpty()) {
        saveFileAs();
    } else {
        saveTo(m_filePath);
    }
}

void Editor::saveFileAs() {
    QString path = QFileDialog::getSaveFileName(this, "Save Map Data", QString(), "INI files (*.ini)");
    if (path.isEmpty()) {
        return;
    }
    saveTo(path);
}

void Editor::saveTo(const QString& path) {
    QSettings set(path, QSettings::IniFormat);
    set.clear();
    for (auto it = m_levels.begin(); it != m_levels.end(); ++it) {
        QString pre = QString("Level-%1").arg(it.key());
        set.setValue(pre + "/blocks", encodeBlocks(it.value().blocks).toBase64());
        set.setValue(pre + "/home", it.value().home);
        set.setValue(pre + "/spawn", it.value().spawn);
    }
    set.sync();
    m_filePath = path;
    setStatus("Saved " + path);
}

void Editor::pushSnapshot() {
    Snapshot s;
    s.level = m_currentLevel;
    s.blocks = m_levels[m_currentLevel].blocks;
    s.home = m_levels[m_currentLevel].home;
    s.spawn = m_levels[m_currentLevel].spawn;
    m_undoStack.append(s);
    m_redoStack.clear();
    updateActions();
}

void Editor::applySnapshot(const Snapshot& s) {
    if (m_currentLevel != s.level) {
        m_currentLevel = s.level;
        for (int i = 0; i < m_levelList->count(); i++) {
            if (m_levelList->item(i)->data(Qt::UserRole).toInt() == s.level) {
                m_levelList->setCurrentRow(i);
                break;
            }
        }
    }
    m_levels[s.level].blocks = s.blocks;
    m_levels[s.level].home = s.home;
    m_levels[s.level].spawn = s.spawn;
    renderMap();
}

void Editor::undo() {
    if (m_undoStack.isEmpty()) {
        return;
    }
    Snapshot s = m_undoStack.takeLast();
    Snapshot cur;
    cur.level = m_currentLevel;
    cur.blocks = m_levels[m_currentLevel].blocks;
    cur.home = m_levels[m_currentLevel].home;
    cur.spawn = m_levels[m_currentLevel].spawn;
    m_redoStack.append(cur);
    applySnapshot(s);
    updateActions();
}

void Editor::redo() {
    if (m_redoStack.isEmpty()) {
        return;
    }
    Snapshot s = m_redoStack.takeLast();
    Snapshot cur;
    cur.level = m_currentLevel;
    cur.blocks = m_levels[m_currentLevel].blocks;
    cur.home = m_levels[m_currentLevel].home;
    cur.spawn = m_levels[m_currentLevel].spawn;
    m_undoStack.append(cur);
    applySnapshot(s);
    updateActions();
}

void Editor::paintCell(const QPoint& cell) {
    if (cell.x() < 0 || cell.x() >= 32 || cell.y() < 0 || cell.y() >= 32) {
        return;
    }
    LevelData& data = m_levels[m_currentLevel];

    if (m_placingHome) {
        QPoint center(cell.x() * 24 + 12, cell.y() * 24 + 12);
        center.setX(qBound(12, center.x(), 756));
        center.setY(qBound(12, center.y(), 756));
        if (data.home.center() != center) {
            pushSnapshot();
            data.home.moveCenter(center);
            renderMap();
        }
        return;
    }

    if (m_placingSpawn) {
        QPoint center(cell.x() * 24 + 12, cell.y() * 24 + 12);
        center.setX(qBound(12, center.x(), 756));
        center.setY(qBound(12, center.y(), 756));
        if (data.spawn.center() != center) {
            pushSnapshot();
            data.spawn.moveCenter(center);
            renderMap();
        }
        return;
    }

    bool changed = false;
    int half = m_brushSize / 2;
    for (int dx = -half; dx <= m_brushSize - 1 - half; dx++) {
        for (int dy = -half; dy <= m_brushSize - 1 - half; dy++) {
            int x = cell.x() + dx, y = cell.y() + dy;
            if (x < 0 || x >= 32 || y < 0 || y >= 32) {
                continue;
            }
            int idx = y * 32 + x;
            if (data.blocks[idx] != m_brush) {
                if (!changed) {
                    pushSnapshot();
                    changed = true;
                }
                data.blocks[idx] = m_brush;
            }
        }
    }
    if (changed) {
        renderMap();
    }
}

void Editor::renderMap() {
    const LevelData& data = m_levels[m_currentLevel];
    QImage img(768, 768, QImage::Format_ARGB32);
    img.fill(Qt::black);
    QPainter p(&img);
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            p.drawImage(i * 24, j * 24, imgBlock[data.blocks[j * 32 + i]]);
        }
    }
    p.drawImage(data.home, imgHome[0]);
    p.drawImage(data.spawn, imgSpawn[0]);
    p.end();
    m_mapItem->setPixmap(QPixmap::fromImage(img));
}

void Editor::updateLevelList() {
    m_levelList->clear();
    m_swapA->clear();
    m_swapB->clear();
    for (auto it = m_levels.begin(); it != m_levels.end(); ++it) {
        QString label = QString("Level %1").arg(it.key());
        QListWidgetItem* item = new QListWidgetItem(label);
        item->setData(Qt::UserRole, it.key());
        m_levelList->addItem(item);
        m_swapA->addItem(label, it.key());
        m_swapB->addItem(label, it.key());
    }
    for (int i = 0; i < m_levelList->count(); i++) {
        if (m_levelList->item(i)->data(Qt::UserRole).toInt() == m_currentLevel) {
            m_levelList->setCurrentRow(i);
            break;
        }
    }
}

void Editor::selectLevel(int row) {
    if (row < 0) {
        return;
    }
    int level = m_levelList->item(row)->data(Qt::UserRole).toInt();
    if (level == m_currentLevel) {
        return;
    }
    m_currentLevel = level;
    renderMap();
}

void Editor::addLevel() {
    int next = m_levels.isEmpty() ? 1 : m_levels.lastKey() + 1;
    m_levels[next] = LevelData();
    updateLevelList();
    m_currentLevel = next;
    for (int i = 0; i < m_levelList->count(); i++) {
        if (m_levelList->item(i)->data(Qt::UserRole).toInt() == next) {
            m_levelList->setCurrentRow(i);
            break;
        }
    }
    renderMap();
    setStatus(QString("Added Level %1").arg(next));
}

void Editor::swapLevels() {
    int a = m_swapA->currentData().toInt();
    int b = m_swapB->currentData().toInt();
    if (a == b) {
        return;
    }
    std::swap(m_levels[a], m_levels[b]);
    updateLevelList();
    renderMap();
    setStatus(QString("Swapped Level %1 and Level %2").arg(a).arg(b));
}

void Editor::generateRandomMap() {
    bool ok = false;
    int seed = QInputDialog::getInt(this, "Random Map", "Seed:", QRandomGenerator::global()->bounded(100000), 0, 2147483647, 1, &ok);
    if (!ok) {
        return;
    }
    pushSnapshot();
    LevelData& data = m_levels[m_currentLevel];
    QRandomGenerator rng(seed);

    enum Theme { ThemeClassic, ThemeSnow, ThemeDesert, ThemeForest };
    Theme theme = (Theme)rng.bounded(4);
    BlockType floor = Ground;
    double brickDensity = 0.20, steelDensity = 0.05, forestDensity = 0.03;
    QString themeName = "Classic";
    switch (theme) {
        case ThemeClassic:
            floor = Ground;
            brickDensity = 0.20;
            steelDensity = 0.05;
            forestDensity = 0.03;
            themeName = "Classic";
            break;
        case ThemeSnow:
            floor = SnowField;
            brickDensity = 0.15;
            steelDensity = 0.03;
            forestDensity = 0.02;
            themeName = "Snow";
            break;
        case ThemeDesert:
            floor = Ground;
            brickDensity = 0.25;
            steelDensity = 0.06;
            forestDensity = 0.0;
            themeName = "Desert";
            break;
        case ThemeForest:
            floor = Ground;
            brickDensity = 0.15;
            steelDensity = 0.04;
            forestDensity = 0.12;
            themeName = "Forest";
            break;
    }

    data.blocks.fill(floor);

    int riverCount = rng.bounded(1, 3);
    for (int r = 0; r < riverCount; r++) {
        bool horizontal = rng.bounded(2) == 0;
        int width = rng.bounded(1, 3);
        if (horizontal) {
            int y = rng.bounded(4, 28);
            int x0 = rng.bounded(2, 6);
            int x1 = rng.bounded(26, 30);
            for (int x = x0; x <= x1; x++) {
                for (int dy = 0; dy < width; dy++) {
                    if (y + dy < 32) {
                        data.blocks[(y + dy) * 32 + x] = River;
                    }
                }
            }
        } else {
            int x = rng.bounded(4, 28);
            int y0 = rng.bounded(2, 6);
            int y1 = rng.bounded(26, 30);
            for (int y = y0; y <= y1; y++) {
                for (int dx = 0; dx < width; dx++) {
                    if (x + dx < 32) {
                        data.blocks[y * 32 + x + dx] = River;
                    }
                }
            }
        }
    }

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            if (data.blocks[y * 32 + x] != floor) {
                continue;
            }
            double r = rng.generateDouble();
            if (r < brickDensity) {
                data.blocks[y * 32 + x] = Brick;
            } else if (r < brickDensity + steelDensity) {
                data.blocks[y * 32 + x] = Steel;
            } else if (r < brickDensity + steelDensity + forestDensity) {
                data.blocks[y * 32 + x] = Forest;
            }
        }
    }

    data.home = QRect(360, 336, 48, 48);
    data.spawn = QRect(360, 24, 48, 48);

    for (int y = 13; y <= 17; y++) {
        for (int x = 14; x <= 18; x++) {
            data.blocks[y * 32 + x] = floor;
        }
    }

    for (int y = 0; y <= 3; y++) {
        for (int x = 14; x <= 18; x++) {
            data.blocks[y * 32 + x] = floor;
        }
    }

    auto passable = [&](int x, int y) {
        if (x < 0 || x >= 32 || y < 0 || y >= 32) {
            return false;
        }
        BlockType t = data.blocks[y * 32 + x];
        return t != Brick && t != Steel && t != River;
    };
    auto reachable = [&]() {
        QPoint start(16, 1), goal(16, 15);
        QVector<bool> visited(1024, false);
        QList<QPoint> queue;
        queue.append(start);
        visited[1 * 32 + 16] = true;
        const int dx[4] = {0, -1, 0, 1};
        const int dy[4] = {-1, 0, 1, 0};
        while (!queue.isEmpty()) {
            QPoint p = queue.takeFirst();
            if (p == goal) {
                return true;
            }
            for (int d = 0; d < 4; d++) {
                int nx = p.x() + dx[d], ny = p.y() + dy[d];
                if (nx < 0 || nx >= 32 || ny < 0 || ny >= 32) {
                    continue;
                }
                if (visited[ny * 32 + nx] || !passable(nx, ny)) {
                    continue;
                }
                visited[ny * 32 + nx] = true;
                queue.append(QPoint(nx, ny));
            }
        }
        return false;
    };
    int guard = 0;
    while (!reachable() && guard < 300) {
        int idx = rng.bounded(1024);
        BlockType t = data.blocks[idx];
        if (t == Brick || t == Steel) {
            data.blocks[idx] = floor;
            guard++;
        }
    }

    renderMap();
    setStatus(QString("Generated random map (seed %1, theme %2)").arg(seed).arg(themeName));
}

void Editor::updateActions() {
    m_undoAction->setEnabled(!m_undoStack.isEmpty());
    m_redoAction->setEnabled(!m_redoStack.isEmpty());
}

void Editor::setStatus(const QString& text) {
    m_statusLabel->setText(text);
}