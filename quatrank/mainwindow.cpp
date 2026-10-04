#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QHBoxLayout>
#include <QSlider>
#include <QWidgetAction>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow), m_transitioning(false) {
    ui->setupUi(this);
    loadResourceImages();
    m_timer = new QTimer(this);
    m_timer->start(1000 / FramesPerSecond);

    m_scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setSceneRect(0, 0, 768, 768);

    m_hudLabel = new QLabel(this);
    m_hudLabel->setIndent(8);
    ui->statusbar->addWidget(m_hudLabel);

    m_boostBar = new QProgressBar(this);
    m_boostBar->setRange(0, 100);
    m_boostBar->setFixedWidth(120);
    m_boostBar->setFixedHeight(14);
    m_boostBar->setTextVisible(false);
    m_boostBar->setVisible(false);
    ui->statusbar->addPermanentWidget(m_boostBar);

    const QString boostStyle = "QProgressBar { border: none; margin-top: 4px; margin-bottom: 4px; margin-right: 8px; } QProgressBar::chunk { background-color: #4caf50; }";
    const QString cooldownStyle = "QProgressBar { border: none; margin-top: 4px; margin-bottom: 4px; margin-right: 8px; } QProgressBar::chunk { background-color: #9e9e9e; }";

    ui->menuMenu->setTitle("Settings");
    QWidgetAction* volumeAction = new QWidgetAction(ui->menuMenu);
    QWidget* volumeWidget = new QWidget;
    QHBoxLayout* volumeLayout = new QHBoxLayout(volumeWidget);
    volumeLayout->setContentsMargins(6, 2, 6, 2);
    volumeLayout->setSpacing(6);
    QLabel* volumeLabel = new QLabel("Volume", volumeWidget);
    QSlider* volumeSlider = new QSlider(Qt::Horizontal, volumeWidget);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(100);
    volumeSlider->setFixedWidth(120);
    QLabel* volumeValue = new QLabel("100%", volumeWidget);
    volumeValue->setFixedWidth(36);
    volumeLayout->addWidget(volumeLabel);
    volumeLayout->addWidget(volumeSlider);
    volumeLayout->addWidget(volumeValue);
    volumeAction->setDefaultWidget(volumeWidget);
    ui->menuMenu->addAction(volumeAction);

    QMenu* cheatsMenu = ui->menuMenu->addMenu("Cheats");
    QAction* cheatIndestructible = cheatsMenu->addAction("Indestructible");
    QAction* cheatInfiniteAmmo = cheatsMenu->addAction("Infinite Ammo");
    QAction* cheatNoCooldown = cheatsMenu->addAction("No Cooldown");
    QAction* cheatBaseShield = cheatsMenu->addAction("Base Shield");
    for (QAction* action : {cheatIndestructible, cheatInfiniteAmmo, cheatNoCooldown, cheatBaseShield}) {
        action->setCheckable(true);
    }

    m_fadeOverlay = new QWidget(ui->graphicsView->parentWidget());
    m_fadeOverlay->setGeometry(ui->graphicsView->geometry());
    m_fadeOverlay->setStyleSheet("background-color: black;");
    m_fadeOverlay->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_fadeEffect = new QGraphicsOpacityEffect(m_fadeOverlay);
    m_fadeEffect->setOpacity(0.0);
    m_fadeOverlay->setGraphicsEffect(m_fadeEffect);
    m_fadeOverlay->raise();
    m_fadeOverlay->setVisible(false);

    m_game = new OriGame(m_scene);
    m_game->init(1);
    updateTitle();
    connect(volumeSlider, &QSlider::valueChanged, this, [this, volumeValue](int value) {
        m_game->setVolume(value / 100.0);
        volumeValue->setText(QString("%1%").arg(value));
    });
    connect(cheatIndestructible, &QAction::toggled, this, [this](bool on) { m_game->setCheatIndestructible(on); });
    connect(cheatInfiniteAmmo, &QAction::toggled, this, [this](bool on) { m_game->setCheatInfiniteAmmo(on); });
    connect(cheatNoCooldown, &QAction::toggled, this, [this](bool on) { m_game->setCheatNoCooldown(on); });
    connect(cheatBaseShield, &QAction::toggled, this, [this](bool on) { m_game->setCheatBaseShield(on); });
    connect(m_timer, &QTimer::timeout, this, [this, boostStyle, cooldownStyle]() {
        QString boostText;
        if (m_game->playerBoosting()) {
            boostText = " | Boosting";
        } else if (m_game->playerBoostCooldown() > 0) {
            boostText = QString(" | Cooldown %1s").arg((m_game->playerBoostCooldown() + FramesPerSecond - 1) / FramesPerSecond);
        }
        m_hudLabel->setText(QString("Level %1 | Enemies %2 | Lives %3 | Rank %4%5%6")
                                .arg(m_game->level())
                                .arg(m_game->restHostileAmt())
                                .arg(m_game->playerBackup())
                                .arg(m_game->playerRank())
                                .arg(m_game->isPaused() ? " | Paused" : "")
                                .arg(boostText));

        const int boostTotal = 3 * FramesPerSecond;
        const int cooldownTotal = 5 * FramesPerSecond;
        if (m_game->playerBoosting()) {
            m_boostBar->setVisible(true);
            m_boostBar->setValue(m_game->playerBoostRemaining() * 100 / boostTotal);
            m_boostBar->setStyleSheet(boostStyle);
        } else if (m_game->playerBoostCooldown() > 0) {
            m_boostBar->setVisible(true);
            m_boostBar->setValue((cooldownTotal - m_game->playerBoostCooldown()) * 100 / cooldownTotal);
            m_boostBar->setStyleSheet(cooldownStyle);
        } else {
            m_boostBar->setVisible(false);
        }

        if (m_transitioning) {
            m_game->update();
            return;
        }
        int result = m_game->update();
        if (result != 0) {
            startTransition(result);
        }
    });
}

void MainWindow::startTransition(int result) {
    m_transitioning = true;
    m_fadeOverlay->setVisible(true);
    m_fadeOverlay->raise();

    QVariantAnimation* fadeOut = new QVariantAnimation(this);
    fadeOut->setStartValue(0.0);
    fadeOut->setEndValue(1.0);
    fadeOut->setDuration(1000);
    fadeOut->setEasingCurve(QEasingCurve::InOutQuad);
    connect(fadeOut, &QVariantAnimation::valueChanged, this, [this](const QVariant& v) { m_fadeEffect->setOpacity(v.toReal()); });
    connect(fadeOut, &QVariantAnimation::finished, this, [this, result, fadeOut]() {
        fadeOut->deleteLater();

        if (result == 1) {
            m_game->init(m_game->level() + 1);
        } else {
            m_game->init(m_game->level());
        }
        updateTitle();

        QVariantAnimation* fadeIn = new QVariantAnimation(this);
        fadeIn->setStartValue(1.0);
        fadeIn->setEndValue(0.0);
        fadeIn->setDuration(300);
        fadeIn->setEasingCurve(QEasingCurve::OutQuad);
        connect(fadeIn, &QVariantAnimation::valueChanged, this, [this](const QVariant& v) { m_fadeEffect->setOpacity(v.toReal()); });
        connect(fadeIn, &QVariantAnimation::finished, this, [this, fadeIn]() {
            fadeIn->deleteLater();
            m_fadeOverlay->setVisible(false);
            m_transitioning = false;
        });
        fadeIn->start();
    });
    fadeOut->start();
}

void MainWindow::updateTitle() {
    setWindowTitle(QString("Quatrank - Level %1").arg(m_game->level()));
}

MainWindow::~MainWindow() {
    delete ui;
    delete m_timer;
    delete m_scene;
    delete m_game;
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    m_game->keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent* event) {
    m_game->keyReleaseEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    exit(0);
}
