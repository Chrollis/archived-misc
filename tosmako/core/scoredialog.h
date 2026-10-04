#ifndef SCOREDIALOG_H
#define SCOREDIALOG_H

#include <QDialog>

namespace Ui {
class ScoreDialog;
}

class ScoreDialog : public QDialog {
    Q_OBJECT

public:
    explicit ScoreDialog(QWidget* parent = nullptr);
    ~ScoreDialog();
    void set_score_to(int score, QString to);

private:
    Ui::ScoreDialog* ui;
};

#endif  // SCOREDIALOG_H
