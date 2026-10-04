#ifndef UTILS_H
#define UTILS_H

#include <QImage>

constexpr int FramesPerPicture = 12;
constexpr int FramesPerSecond = 60;
constexpr int CycleIndex[4] = {0, 1, 2, 1};

QString getResourcePath(const QString& suffix);
QImage whiteSlashTexture(const QImage& src);
QImage operator+(const QImage& background, const QImage& frontground);

extern QImage imgBlock[6];
extern QImage imgHome[5];
extern QImage imgSpawn[3];
extern QImage imgLoot[4];
extern QImage imgPlayer[3];
extern QImage imgHostile[4];
extern QImage imgFlicker[3];
extern QImage imgBullet[2];
extern QImage imgBang[5];

void loadResourceImages();

enum BlockType { Ground, Brick, Steel, River, Forest, SnowField };
enum LootType { Pistol, Shell, Star, Backup };
enum Direction { Up, Left, Down, Right };
enum ItemLevel { LAtlas, LLoot, LTank, LForest, LBullet };

#endif  // UTILS_H
