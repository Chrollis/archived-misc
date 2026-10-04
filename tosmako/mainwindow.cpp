#include "mainwindow.h"

#include <QAction>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>

#include "./ui_mainwindow.h"
#include "coworkgame.h"
#include "pregame.h"
#include "pvpgame.h"
#include "scoredialog.h"
#include "singlegame.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->setWindowTitle("Tosmako");
    this->setWindowIcon(QIcon(":/icon/assets/tomato.ico"));

    scoreLabel = new QLabel();
    statusBar()->addPermanentWidget(scoreLabel);

    init_scenes();
    m_timer = new QTimer(this);
    t_timer = new QTimer(this);

    connect(m_timer, &QTimer::timeout, this, &MainWindow::game_snake);
    connect(t_timer, &QTimer::timeout, this, &MainWindow::game_tomato);

    connect(ui->singleBtn, &QPushButton::clicked, this, [&]() {
        if (m_game->score() > 1000) {
            switchToPlaying(Pre);
        } else {
            switchToPlaying(Single);
        }
    });
    connect(ui->coworkBtn, &QPushButton::clicked, this, [&]() { switchToPlaying(Cowork); });
    connect(ui->pvpBtn, &QPushButton::clicked, this, [&]() { switchToPlaying(PVP); });
    connect(ui->dfctBtn, &QPushButton::clicked, this, &MainWindow::dfct_dialog);

    connect(ui->exitBtn, &QPushButton::clicked, this, [&]() { exit(0); });
    connect(ui->continueBtn, &QPushButton::clicked, this, &MainWindow::back_playing);
    connect(ui->restartBtn, &QPushButton::clicked, this, [&]() { switchToPlaying(m_mode); });
    connect(ui->backBtn, &QPushButton::clicked, this, &MainWindow::back_menu);
    connect(ui->backBtn_2, &QPushButton::clicked, this, &MainWindow::menu);

    connect(ui->dfctAction, &QAction::triggered, this, &MainWindow::dfct_dialog);
    connect(ui->restartAction, &QAction::triggered, this, &MainWindow::back_restart);
    connect(ui->menuAction, &QAction::triggered, this, &MainWindow::back_menu);
    connect(ui->exitAction, &QAction::triggered, this, &MainWindow::back_exit);

    ui->stackedWidget->setCurrentIndex(Menu);
    m_timer->start(100);
    t_timer->start(500);
    ui->menuAction->setEnabled(0);
    ui->pauseAction->setEnabled(0);
    ui->dfctAction->setEnabled(1);
    ui->restartAction->setEnabled(0);
    load_score();
    ui->singleRank->setText(
        QString("单人排名\n第一名 %1\n第二名 %2\n第三名 %3\n第四名 %4\n第五名 %5").arg(single_scores[0]).arg(single_scores[1]).arg(single_scores[2]).arg(single_scores[3]).arg(single_scores[4]));
    ui->singleRank_2->setText(
        QString("单人排名\n第六名 %1\n第七名 %2\n第八名 %3\n第九名 %4\n第十名 %5").arg(single_scores[5]).arg(single_scores[6]).arg(single_scores[7]).arg(single_scores[8]).arg(single_scores[9]));
    ui->coworkLabel->setText(
        QString("双人排名\n第一名 %1\n第二名 %2\n第三名 %3\n第四名 %4\n第五名 %5").arg(cowork_scores[0]).arg(cowork_scores[1]).arg(cowork_scores[2]).arg(cowork_scores[3]).arg(cowork_scores[4]));
    ui->pvpLabel->setText(QString("对战排名\n第一名 %1\n第二名 %2\n第三名 %3\n第四名 %4\n第五名 %5").arg(pvp_scores[0]).arg(pvp_scores[1]).arg(pvp_scores[2]).arg(pvp_scores[3]).arg(pvp_scores[4]));
    init_sceneGrid(scene[Menu]);
    m_game = new PreGame(scene[Menu]);
}

MainWindow::~MainWindow() {
    delete ui;
    delete m_game;
    delete m_timer;
    delete t_timer;
    delete scoreLabel;
    delete scene[Menu];
    delete scene[Playing];
    delete scene[Paused];
    delete scene[GameOver];
}

void MainWindow::init_scenes() {
    scene[Menu] = new QGraphicsScene(this);
    ui->menuView->setScene(scene[Menu]);
    ui->menuView->setRenderHint(QPainter::Antialiasing);
    ui->menuView->setSceneRect(-gridEdge, -gridEdge, 800, 800);

    scene[Playing] = new QGraphicsScene(this);
    ui->playingView->setScene(scene[Playing]);
    ui->playingView->setRenderHint(QPainter::Antialiasing);
    ui->playingView->setSceneRect(-gridEdge, -gridEdge, 800, 800);

    scene[Paused] = new QGraphicsScene(this);
    ui->pausedView->setScene(scene[Paused]);
    ui->pausedView->setRenderHint(QPainter::Antialiasing);

    scene[GameOver] = new QGraphicsScene(this);
    ui->gameOverView->setScene(scene[GameOver]);
    ui->gameOverView->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::init_sceneGrid(QGraphicsScene* scene) {
    int size = snakeSize;
    int width = gridSize * snakeSize;
    int height = gridSize * snakeSize;
    QPen gridPen(QColor(0xDDDDDD), 1);
    gridPen.setStyle(Qt::DotLine);
    for (int x = 0; x <= gridSize; x++) {
        QGraphicsLineItem* line = new QGraphicsLineItem(x * size, 0, x * size, height);
        line->setPen(gridPen);
        scene->addItem(line);
    }
    for (int y = 0; y <= gridSize; y++) {
        QGraphicsLineItem* line = new QGraphicsLineItem(0, y * size, width, y * size);
        line->setPen(gridPen);
        scene->addItem(line);
    }
}

void MainWindow::switchToMenu() {
    delete m_game;
    menu();
}

void MainWindow::switchToPlaying(Mode mode) {
    delete m_game;
    m_mode = mode;
    ui->stackedWidget->setCurrentIndex(Playing);
    m_timer->start(m_mode == Pre ? 100 : 200 - 50 * m_dfct);
    t_timer->start(2000 - 500 * m_mode);
    ui->menuAction->setEnabled(1);
    ui->restartAction->setEnabled(1);
    ui->pauseAction->setEnabled(1);
    ui->dfctAction->setEnabled(0);
    scene[Menu]->clear();
    init_sceneGrid(scene[Playing]);
    switch (m_mode) {
        case Single:
            m_game = new SingleGame(scene[Playing]);
            break;
        case Cowork:
            m_game = new CoworkGame(scene[Playing]);
            break;
        case PVP:
            m_game = new PVPGame(scene[Playing]);
            break;
        case Pre:
            m_game = new PreGame(scene[Playing]);
            break;
    }
}

void MainWindow::switchToPaused() {
    m_timer->stop();
    t_timer->stop();
    ui->stackedWidget->setCurrentIndex(Paused);
    ui->menuAction->setEnabled(1);
    ui->restartAction->setEnabled(1);
    ui->pauseAction->setEnabled(0);
    ui->dfctAction->setEnabled(0);
    screenshot = ui->playingView->grab();
    QImage image = screenshot.toImage().convertToFormat(QImage::Format_ARGB32);
    QImage result(image.size(), QImage::Format_ARGB32);
    int red, green, blue, alpha;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QRgb pixel = image.pixel(x, y);
            red = qRed(pixel) / 2;
            green = qGreen(pixel) / 2;
            blue = qBlue(pixel) / 2;
            alpha = qAlpha(pixel);
            result.setPixel(x, y, qRgba(red, green, blue, alpha));
        }
    }
    screenshot = QPixmap::fromImage(result).scaled(screenshot.size() / screenshot.devicePixelRatio());
    ui->screenshotLabel->setPixmap(screenshot);
}

void MainWindow::switchToGameOver() {
    ui->stackedWidget->setCurrentIndex(GameOver);
    save_score(m_game->score());
    m_timer->stop();
    t_timer->stop();
    ui->menuAction->setEnabled(1);
    ui->pauseAction->setEnabled(0);
    ui->dfctAction->setEnabled(1);
    ui->restartAction->setEnabled(1);
    screenshot = ui->playingView->grab();
    QImage image = screenshot.toImage().convertToFormat(QImage::Format_ARGB32);
    QImage result(image.size(), QImage::Format_ARGB32);
    int red, green, blue, alpha;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QRgb pixel = image.pixel(x, y);
            red = qRed(pixel) / 5 * 4;
            green = qGreen(pixel) / 2;
            blue = qBlue(pixel) / 2;
            alpha = qAlpha(pixel);
            result.setPixel(x, y, qRgba(red, green, blue, alpha));
        }
    }
    screenshot = QPixmap::fromImage(result).scaled(screenshot.size() / screenshot.devicePixelRatio());
    ui->screenshotLabel_2->setPixmap(screenshot);
    scoreLabel->setText("");
    switch (m_mode) {
        case Single:
            ui->sumLabel->setText(QString("你得了%1分，于主菜单查看排名").arg(m_game->score()));
            break;
        case Cowork:
            ui->sumLabel->setText(QString("你们得了%1分，于主菜单查看排名").arg(m_game->score()));
            break;
        case PVP:
            ui->sumLabel->setText(QString("赢家得了%1分，于主菜单查看排名").arg(m_game->score()));
            break;
        case Pre:
            break;
    }
}

void MainWindow::dfct_dialog() {
    DifficultyDialog* dfct_dialog = new DifficultyDialog;

    dfct_dialog->set_current_difficulty(m_dfct);
    if (dfct_dialog->exec() == QDialog::Accepted) {
        m_dfct = dfct_dialog->get_difficulty();
    }
    delete dfct_dialog;
}

void MainWindow::back_menu() {
    if (ui->stackedWidget->currentIndex() == Playing) {
        switchToPaused();
    }
    if (ui->stackedWidget->currentIndex() != GameOver) {
        ScoreDialog* score_dialog = new ScoreDialog;

        score_dialog->set_score_to(m_game->score(), "返回主菜单");
        if (score_dialog->exec() == QDialog::Accepted) {
            save_score(m_game->score());
            switchToMenu();
        }
        delete score_dialog;
    }
}

void MainWindow::back_exit() {
    if (ui->stackedWidget->currentIndex() == Playing) {
        switchToPaused();
    }
    if (ui->stackedWidget->currentIndex() != Menu && ui->stackedWidget->currentIndex() != GameOver) {
        ScoreDialog* score_dialog = new ScoreDialog;

        score_dialog->set_score_to(m_game->score(), "退出");
        if (score_dialog->exec() == QDialog::Accepted) {
            save_score(m_game->score());
            delete score_dialog;

            exit(0);
        }
        delete score_dialog;

    } else {
        exit(0);
    }
}

void MainWindow::back_restart() {
    if (ui->stackedWidget->currentIndex() == Playing) {
        switchToPaused();
    }
    ScoreDialog* score_dialog = new ScoreDialog;

    score_dialog->set_score_to(m_game->score(), "重新开始");
    if (score_dialog->exec() == QDialog::Accepted) {
        save_score(m_game->score());
        switchToPlaying(m_mode);
    }
    delete score_dialog;
}

void MainWindow::back_playing() {
    ui->stackedWidget->setCurrentIndex(Playing);
    m_timer->start();
    t_timer->start();
    ui->menuAction->setEnabled(1);
    ui->restartAction->setEnabled(1);
    ui->pauseAction->setEnabled(1);
    ui->dfctAction->setEnabled(0);
}

void MainWindow::menu() {
    ui->stackedWidget->setCurrentIndex(Menu);
    m_timer->start(100);
    t_timer->start(500);
    ui->menuAction->setEnabled(0);
    ui->pauseAction->setEnabled(0);
    ui->dfctAction->setEnabled(1);
    ui->restartAction->setEnabled(0);
    scoreLabel->setText("");
    load_score();
    ui->singleRank->setText(
        QString("单人排名\n第一名 %1\n第二名 %2\n第三名 %3\n第四名 %4\n第五名 %5").arg(single_scores[0]).arg(single_scores[1]).arg(single_scores[2]).arg(single_scores[3]).arg(single_scores[4]));
    ui->singleRank_2->setText(
        QString("单人排名\n第六名 %1\n第七名 %2\n第八名 %3\n第九名 %4\n第十名 %5").arg(single_scores[5]).arg(single_scores[6]).arg(single_scores[7]).arg(single_scores[8]).arg(single_scores[9]));
    ui->coworkLabel->setText(
        QString("双人排名\n第一名 %1\n第二名 %2\n第三名 %3\n第四名 %4\n第五名 %5").arg(cowork_scores[0]).arg(cowork_scores[1]).arg(cowork_scores[2]).arg(cowork_scores[3]).arg(cowork_scores[4]));
    ui->pvpLabel->setText(QString("对战排名\n第一名 %1\n第二名 %2\n第三名 %3\n第四名 %4\n第五名 %5").arg(pvp_scores[0]).arg(pvp_scores[1]).arg(pvp_scores[2]).arg(pvp_scores[3]).arg(pvp_scores[4]));
    scene[Playing]->clear();
    init_sceneGrid(scene[Menu]);
    m_mode = Pre;
    m_game = new PreGame(scene[Menu]);
}

void MainWindow::load_score() {
    QSettings settings("config.ini", QSettings::IniFormat);
    for (int i = 0; i < 10; i++) {
        single_scores[i] = settings.value(QString("single/rank%1").arg(i)).toInt();
    }
    for (int i = 0; i < 5; i++) {
        cowork_scores[i] = settings.value(QString("cowork/rank%1").arg(i)).toInt();
    }
    for (int i = 0; i < 5; i++) {
        pvp_scores[i] = settings.value(QString("pvp/rank%1").arg(i)).toInt();
    }
}

void MainWindow::save_score(int score) {
    QSettings settings("config.ini", QSettings::IniFormat);
    int i = 0;
    switch (m_mode) {
        case Single:
        case Pre:
            for (; i < 10; i++) {
                if (score >= single_scores[i]) {
                    break;
                }
            }
            for (int j = 9; j > i; j--) {
                settings.setValue(QString("single/rank%1").arg(j), single_scores[j - 1]);
            }
            settings.setValue(QString("single/rank%1").arg(i), score);
            break;
        case Cowork:
            for (; i < 5; i++) {
                if (score >= cowork_scores[i]) {
                    break;
                }
            }
            for (int j = 4; j > i; j--) {
                settings.setValue(QString("cowork/rank%1").arg(j), cowork_scores[j - 1]);
            }
            settings.setValue(QString("cowork/rank%1").arg(i), score);
            break;
        case PVP:
            for (; i < 5; i++) {
                if (score >= pvp_scores[i]) {
                    break;
                }
            }
            for (int j = 4; j > i; j--) {
                settings.setValue(QString("pvp/rank%1").arg(j), pvp_scores[j - 1]);
            }
            settings.setValue(QString("pvp/rank%1").arg(i), score);
            break;
    }
}

void MainWindow::game_snake() {
    m_game->update();
    m_game->update_tomato();
    if (m_game->is_over()) {
        if (m_mode != Pre) {
            switchToGameOver();
        } else {
            if (ui->stackedWidget->currentIndex() == Menu) {
                switchToMenu();
            } else if (ui->stackedWidget->currentIndex() == Playing) {
                m_mode = Single;
                switchToGameOver();
            }
        }
    } else {
        if (m_game->is_up()) {
            game_speed_up();
        }
        const QString dfct_string[4] = {"简单", "普通", "困难", "极限"};
        if (ui->stackedWidget->currentIndex() != GameOver) {
            switch (m_mode) {
                case Single:
                    scoreLabel->setText(QString("你有%1分，当前难度为：").arg(m_game->score()) + dfct_string[m_dfct]);
                    break;
                case Cowork:
                    scoreLabel->setText(QString("你们有%1分，当前难度为：").arg(m_game->score()) + dfct_string[m_dfct]);
                    break;
                case PVP:
                    scoreLabel->setText(QString("最高分有%1分，当前难度为：").arg(m_game->score()) + dfct_string[m_dfct]);
                    break;
                case Pre:
                    scoreLabel->setText(QString("已获得%1分").arg(m_game->score()));
                    break;
            }
        }
    }
}

void MainWindow::game_speed_up() {
    if (m_mode != Pre) {
        m_timer->setInterval(qMax(int(10 * pow(2, 3 - m_dfct)), m_timer->interval() - 5));
    }
}

void MainWindow::game_tomato() {
    m_game->create_tomato();
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    m_game->keyPressEvent(event);
    if (ui->stackedWidget->currentIndex() == Playing && event->key() == Qt::Key_Space) {
        switchToPaused();
    }
    /*if (event->key() == Qt::Key::Key_M) {
      if (m_timer->isActive()) {
          m_timer->stop();
      } else {
          m_timer->start();
      }
  } else if (event->key() == Qt::Key::Key_N) {
      m_game->create_tomato();
  }*/
}

void MainWindow::mousePressEvent(QMouseEvent* event) {
    m_game->mousePressEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (ui->stackedWidget->currentIndex() == Playing) {
        switchToPaused();
    }
    if (ui->stackedWidget->currentIndex() != Menu || ui->stackedWidget->currentIndex() == GameOver) {
        ScoreDialog* score_dialog = new ScoreDialog;

        score_dialog->set_score_to(m_game->score(), "退出");
        if (score_dialog->exec() == QDialog::Accepted) {
            save_score(m_game->score());
        } else {
            event->ignore();
        }
        delete score_dialog;
    }
}