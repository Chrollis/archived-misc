#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>
#include <QTimer>
#include <QVariantAnimation>
#include "origame.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    Ui::MainWindow* ui;
    QTimer* m_timer;
    QGraphicsScene* m_scene;
    OriGame* m_game;
    QLabel* m_hudLabel;
    QProgressBar* m_boostBar;
    QWidget* m_fadeOverlay;
    QGraphicsOpacityEffect* m_fadeEffect;
    bool m_transitioning;
    void updateTitle();
    void startTransition(int result);
};
#endif  // MAINWINDOW_H
