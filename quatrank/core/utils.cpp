#include "utils.h"

#include <QCoreApplication>
#include <QDir>

QString getResourcePath(const QString& suffix) {
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("assets/") + suffix);
}

QImage whiteSlashTexture(const QImage& src) {
    QImage img = src.convertToFormat(QImage::Format_ARGB32);
    for (int i = 0; i < img.width(); i++) {
        for (int j = 0; j < img.height(); j++) {
            if (((i + j) / 6) % 2) {
                QRgb pixel = img.pixel(i, j);
                int alpha = qAlpha(pixel);
                int red = qRed(pixel);
                int green = qGreen(pixel);
                int blue = qBlue(pixel);
                int grey = qMin((int)(0.299 * red + 0.587 * green + 0.114 * blue) + 64, 255);
                img.setPixel(i, j, qRgba(grey, grey, grey, alpha));
            }
        }
    }
    return img;
}

QImage operator+(const QImage& background, const QImage& frontground) {
    QImage img = background.convertToFormat(QImage::Format_ARGB32);
    if (background.size() == frontground.size()) {
        for (int i = 0; i < img.width(); i++) {
            for (int j = 0; j < img.height(); j++) {
                QRgb frontPixel = frontground.pixel(i, j);
                QRgb backPixel = background.pixel(i, j);
                double alphaFrontPixel = (double)qAlpha(frontPixel) / 255;
                double alphaBackPixel = (double)qAlpha(backPixel) / 255;
                double alpha = alphaFrontPixel + alphaBackPixel * (1 - alphaFrontPixel);
                if (alpha == 0) {
                    img.setPixel(i, j, qRgba(0, 0, 0, 0));
                } else {
                    int r = (qRed(frontPixel) * alphaFrontPixel + qRed(backPixel) * alphaBackPixel * (1 - alphaFrontPixel)) / alpha;
                    int g = (qGreen(frontPixel) * alphaFrontPixel + qGreen(backPixel) * alphaBackPixel * (1 - alphaFrontPixel)) / alpha;
                    int b = (qBlue(frontPixel) * alphaFrontPixel + qBlue(backPixel) * alphaBackPixel * (1 - alphaFrontPixel)) / alpha;
                    img.setPixel(i, j, qRgba(std::min(r, 255), std::min(g, 255), std::min(b, 255), (int)(alpha * 255)));
                }
            }
        }
    }
    return img;
}

QImage imgBlock[6];
QImage imgHome[5];
QImage imgSpawn[3];
QImage imgLoot[4];
QImage imgPlayer[3];
QImage imgHostile[4];
QImage imgFlicker[3];
QImage imgBullet[2];
QImage imgBang[5];

void loadResourceImages() {
    QImage temp;
    temp.load(getResourcePath("sprites/blocks.png"));
    for (int i = 0; i < 6; i++) {
        imgBlock[i] = temp.copy(i * 24, 0, 24, 24);
    }
    temp.load(getResourcePath("sprites/homes.png"));
    for (int i = 0; i < 5; i++) {
        imgHome[i] = temp.copy(i * 44, 0, 44, 44);
    }
    temp.load(getResourcePath("sprites/spawns.png"));
    for (int i = 0; i < 3; i++) {
        imgSpawn[i] = temp.copy(i * 44, 0, 44, 44);
    }
    temp.load(getResourcePath("sprites/loots.png"));
    for (int i = 0; i < 4; i++) {
        imgLoot[i] = temp.copy(i * 32, 0, 32, 32);
    }
    temp.load(getResourcePath("sprites/tanks.png"));
    for (int i = 0; i < 3; i++) {
        imgPlayer[i] = temp.copy(i * 44, 44, 44, 44);
    }
    for (int i = 0; i < 4; i++) {
        imgHostile[i] = temp.copy(i * 44, 0, 44, 44);
    }
    temp.load(getResourcePath("sprites/flickers.png"));
    for (int i = 0; i < 3; i++) {
        imgFlicker[i] = temp.copy(i * 44, 0, 44, 44);
    }
    temp.load(getResourcePath("sprites/bullet.png"));
    for (int i = 0; i < 2; i++) {
        imgBullet[i] = temp.copy(i * 12, 0, 12, 12);
    }
    temp.load(getResourcePath("sprites/bangs.png"));
    for (int i = 0; i < 5; i++) {
        imgBang[i] = temp.copy(i * 96, 0, 96, 96);
    }
}
