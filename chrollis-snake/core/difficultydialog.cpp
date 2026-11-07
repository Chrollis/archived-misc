#include "difficultydialog.h"
#include "ui_difficultydialog.h"

DifficultyDialog::DifficultyDialog(QWidget* parent) : QDialog(parent), ui(new Ui::DifficultyDialog) {
    ui->setupUi(this);
    this->setWindowTitle("难度选择");
}

DifficultyDialog::~DifficultyDialog() {
    delete ui;
}

void DifficultyDialog::set_current_difficulty(Difficulty dfct) {
    QString dfct_name;
    switch (dfct) {
        case Easy:
            dfct_name = "【简单】";
            break;
        case Normal:
            dfct_name = "【普通】";
            break;
        case Difficult:
            dfct_name = "【困难】";
            break;
        case Incredible:
            dfct_name = "【极限】";
            break;
    }

    ui->current->setText("当前难度为：" + dfct_name);
    ui->comboBox->setCurrentIndex(dfct);
}

Difficulty DifficultyDialog::get_difficulty() {
    return (Difficulty)ui->comboBox->currentIndex();
}
