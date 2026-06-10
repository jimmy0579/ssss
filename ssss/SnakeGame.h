#ifndef SNAKEGAME_H
#define SNAKEGAME_H

#include <deque> //這裡用來存蛇的身體。
#include <vector>
#include <string>
#include <random>
#include <conio.h> //這兩個可以偵測鍵盤輸入。
#include <windows.h>

using namespace std;

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};
//它用來表示蛇目前的移動方向。
struct Point {
    int x;
    int y;

    bool equals(const Point& other) const;
};
//它用來表示座標。
class Snake {
private:
    deque<Point> body;//用 deque 儲存蛇的身體。每一個 Point 都代表蛇身上的一格。
    Direction direction;//儲存蛇目前的方向。
    bool needGrow;

public:
    Snake(int startX, int startY);
    //startX 和 startY 是蛇一開始的位置。
    void reset(int startX, int startY);
    //重新設定蛇的位置和狀態。遊戲重新開始時會用到。
    Point getHead() const;
    const deque<Point>& getBody() const;

    void changeDirection(Direction newDirection);
    //改變蛇的移動方向。
    void move();
    void grow();

    bool hitSelf() const;//判斷蛇有沒有撞到自己。
    bool occupies(Point p) const;//這個功能是為了避免食物生成在蛇身上。
};

class Food {
private:
    Point position;//食物目前的位置。
    mt19937 randomEngine;
    //隨機數引擎。用來產生隨機座標。
public:
    Food();

    void spawn(int width, int height, const Snake& snake);//產生新的食物位置。
    Point getPosition() const;
    //取得食物目前的位置。
};

class Game {
private:
    const int width = 40;
    const int height = 20;

    Snake snake;
    Food food;

    bool gameOver;
    int score;
    int speed;

public:
    Game();
    void run();

private:
    void restart();
    bool handleInput();
    void update();
    void draw();
    void showGameOverMessage();

    void setCursorToTopLeft();
    void hideCursor();
};

#endif
