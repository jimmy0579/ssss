#include "SnakeGame.h"
#include <iostream>

using namespace std;

bool Point::equals(const Point& other) const {
    return x == other.x && y == other.y;
}
//實作 Point 裡面的 equals() 函式。
Snake::Snake(int startX, int startY) {
    reset(startX, startY);
}
void Snake::reset(int startX, int startY) {
    body.clear();
    //重新開始遊戲時，需要先把舊蛇刪掉。
    body.push_back({ startX, startY });
    body.push_back({ startX - 1, startY });
    body.push_back({ startX - 2, startY });

    direction = RIGHT;//設定蛇一開始往右移動。
    needGrow = false;
}

Point Snake::getHead() const {
    return body.front();
}
//取得蛇頭位置。
const deque<Point>& Snake::getBody() const {
    return body;
}

void Snake::changeDirection(Direction newDirection) {//改變蛇的方向。newDirection 是玩家按鍵後的新方向。
    if (direction == UP && newDirection == DOWN) return;
    if (direction == DOWN && newDirection == UP) return;
    if (direction == LEFT && newDirection == RIGHT) return;
    if (direction == RIGHT && newDirection == LEFT) return;

    direction = newDirection;//如果不是反方向，就把目前方向改成新方向。
}

void Snake::move() {
    Point newHead = getHead();//先複製目前蛇頭位置。接下來會根據方向改變它的位置。

    switch (direction) {
    case UP:
        newHead.y--;
        break;
    case DOWN:
        newHead.y++;
        break;
    case LEFT:
        newHead.x--;
        break;
    case RIGHT:
        newHead.x++;
        break;
    }

    body.push_front(newHead);

    if (needGrow) {
        needGrow = false;
    }
    else {
        body.pop_back();
    }
}

void Snake::grow() {
    needGrow = true;
}

bool Snake::hitSelf() const {
    Point head = getHead();

    for (size_t i = 1; i < body.size(); i++) {
        if (body[i].equals(head)) {
            return true;
        }
    }

    return false;
}

bool Snake::occupies(Point p) const {
    for (const Point& part : body) {
        if (part.equals(p)) {
            return true;
        }
    }

    return false;
}

Food::Food() {
    random_device rd;
    randomEngine = mt19937(rd());
    position = { 1, 1 };
}

void Food::spawn(int width, int height, const Snake& snake) {
    uniform_int_distribution<int> xDist(1, width - 2);
    uniform_int_distribution<int> yDist(1, height - 2);

    do {
        position.x = xDist(randomEngine);
        position.y = yDist(randomEngine);
    } while (snake.occupies(position));
}

Point Food::getPosition() const {
    return position;
}

Game::Game() : snake(width / 2, height / 2) {
    hideCursor();
    restart();
}

void Game::run() {
    draw();

    while (true) {
        if (!gameOver) {
            if (!handleInput()) {
                break;
            }

            update();
            draw();

            Sleep(speed);
        }
        else {
            showGameOverMessage();

            int key = _getch();

            if (key == 'r' || key == 'R') {
                restart();
                draw();
            }
            else if (key == 'q' || key == 'Q') {
                break;
            }
        }
    }
}

void Game::restart() {
    snake.reset(width / 2, height / 2);
    food.spawn(width, height, snake);

    gameOver = false;
    score = 0;

    speed = 230;
}

bool Game::handleInput() {
    if (_kbhit()) {
        int key = _getch();

        if (key == 224 || key == 0) {
            key = _getch();

            switch (key) {
            case 72:
                snake.changeDirection(UP);
                break;
            case 80:
                snake.changeDirection(DOWN);
                break;
            case 75:
                snake.changeDirection(LEFT);
                break;
            case 77:
                snake.changeDirection(RIGHT);
                break;
            }
        }
        else {
            switch (key) {
            case 'w':
            case 'W':
                snake.changeDirection(UP);
                break;
            case 's':
            case 'S':
                snake.changeDirection(DOWN);
                break;
            case 'a':
            case 'A':
                snake.changeDirection(LEFT);
                break;
            case 'd':
            case 'D':
                snake.changeDirection(RIGHT);
                break;
            case 'q':
            case 'Q':
                return false;
            }
        }
    }

    return true;
}

void Game::update() {
    snake.move();

    Point head = snake.getHead();

    if (head.x <= 0 || head.x >= width - 1 || head.y <= 0 || head.y >= height - 1) {
        gameOver = true;
        return;
    }

    if (snake.hitSelf()) {
        gameOver = true;
        return;
    }

    if (head.equals(food.getPosition())) {
        snake.grow();
        score += 10;
        food.spawn(width, height, snake);

        if (score % 50 == 0 && speed > 130) {
            speed -= 10;
        }
    }
}

void Game::draw() {
    setCursorToTopLeft();

    vector<string> screen(height, string(width, ' '));

    for (int x = 0; x < width; x++) {
        screen[0][x] = '#';
        screen[height - 1][x] = '#';
    }

    for (int y = 0; y < height; y++) {
        screen[y][0] = '#';
        screen[y][width - 1] = '#';
    }

    Point foodPos = food.getPosition();
    screen[foodPos.y][foodPos.x] = '@';

    const deque<Point>& body = snake.getBody();

    for (size_t i = 0; i < body.size(); i++) {
        Point p = body[i];

        if (p.x >= 0 && p.x < width && p.y >= 0 && p.y < height) {
            if (i == 0) {
                screen[p.y][p.x] = 'O';
            }
            else {
                screen[p.y][p.x] = 'o';
            }
        }
    }

    for (int y = 0; y < height; y++) {
        cout << screen[y] << '\n';
    }

    cout << "Score: " << score << "\n";
    cout << "Control: W A S D or Arrow Keys | Q: Quit\n";
}

void Game::showGameOverMessage() {
    cout << "\n";
    cout << "Game Over!\n";
    cout << "Final Score: " << score << "\n";
    cout << "Press R to restart, or Q to quit.\n";
}

void Game::setCursorToTopLeft() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD position = { 0, 0 };
    SetConsoleCursorPosition(console, position);
}

void Game::hideCursor() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(console, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(console, &cursorInfo);
}
