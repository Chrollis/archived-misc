#include "editor.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    Editor editor;
    editor.show();
    return a.exec();
}