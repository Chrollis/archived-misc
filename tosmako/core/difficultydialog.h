#ifndef DIFFICULTYDIALOG_H
#define DIFFICULTYDIALOG_H

#include <QDialog>

namespace Ui {
class DifficultyDialog;
}

enum Difficulty { Easy, Normal, Difficult, Incredible };

class DifficultyDialog : public QDialog {
    Q_OBJECT

public:
    explicit DifficultyDialog(QWidget* parent = nullptr);
    ~DifficultyDialog();
    void set_current_difficulty(Difficulty dfct);
    Difficulty get_difficulty();

private:
    Ui::DifficultyDialog* ui;
};

#endif  // DIFFICULTYDIALOG_H
