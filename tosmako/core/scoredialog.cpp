#include "scoredialog.h"
#include "ui_scoredialog.h"

ScoreDialog::ScoreDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ScoreDialog) {
    ui->setupUi(this);
    this->setWindowTitle("保存分数");
}

ScoreDialog::~ScoreDialog() {
    delete ui;
}

void ScoreDialog::set_score_to(int score, QString to) {
    ui->label->setText(QString("您的得分为：%1\n确认以当前分数结算并%2吗？").arg(score).arg(to));
}
