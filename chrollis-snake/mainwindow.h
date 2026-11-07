#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QLabel>
#include <QMainWindow>
#include <QTimer>

#include "difficultydialog.h"
#include "game.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

enum Scene { Menu, Playing, Paused, GameOver };
enum Mode { Single, Cowork, PVP, Pre };

class MainWindow : public QMainWindow {
    Q_OBJECT
private:
    Mode m_mode = Pre;
    Difficulty m_dfct = Normal;

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void init_scenes();
    void init_sceneGrid(QGraphicsScene* scene);

private:
    void switchToMenu();
    void switchToPlaying(Mode mode);
    void switchToPaused();
    void switchToGameOver();

private:
    void dfct_dialog();
    void load_score();
    void save_score(int score);
    void game_snake();
    void game_speed_up();
    void game_tomato();

private:
    void back_menu();
    void back_exit();
    void back_restart();
    void back_playing();
    void menu();
    void guide();
    void about();

private:
    Ui::MainWindow* ui;
    QGraphicsScene* scene[4];
    QTimer* m_timer;
    QTimer* t_timer;
    QLabel* scoreLabel;
    QPixmap screenshot;

    Game* m_game;
    int single_scores[10];
    int cowork_scores[5];
    int pvp_scores[5];

private:
    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void closeEvent(QCloseEvent* event);
};
#endif  // MAINWINDOW_H
