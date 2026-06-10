#include "SnakeGame.h"
#include <iostream>

using namespace std;

// 比較兩個座標是否位於同一格。
bool Point::equals(const Point& other) const {
    return x == other.x && y == other.y;
}

// 建立蛇時，交由 reset() 統一設定初始狀態。
Snake::Snake(int startX, int startY) {
    reset(startX, startY);
}

// 將蛇重設為長度 3，蛇頭位於指定座標，並朝右移動。
void Snake::reset(int startX, int startY) {
    body.clear();
    body.push_back({ startX, startY });
    body.push_back({ startX - 1, startY });
    body.push_back({ startX - 2, startY });

    direction = RIGHT;
    needGrow = false;
}

// deque 的第一格永遠代表蛇頭。
Point Snake::getHead() const {
    return body.front();
}

// 回傳唯讀參考，讓畫面可以讀取蛇身而不複製整個 deque。
const deque<Point>& Snake::getBody() const {
    return body;
}

void Snake::changeDirection(Direction newDirection) {
    // 禁止直接反向，避免蛇頭下一步立刻撞上第二節身體。
    if (direction == UP && newDirection == DOWN) return;
    if (direction == DOWN && newDirection == UP) return;
    if (direction == LEFT && newDirection == RIGHT) return;
    if (direction == RIGHT && newDirection == LEFT) return;

    direction = newDirection;
}

void Snake::move() {
    // 從目前蛇頭複製出下一步的座標。
    Point newHead = getHead();

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

    // 把新座標加入最前端，成為新的蛇頭。
    body.push_front(newHead);

    // 吃到食物時保留蛇尾，蛇身便會增加一格。
    if (needGrow) {
        needGrow = false;
    }
    else {
        body.pop_back();
    }
}

void Snake::grow() {
    // 延後到下一次 move() 才增長，避免移動規則分散在多個函式。
    needGrow = true;
}

bool Snake::hitSelf() const {
    Point head = getHead();

    // 從第二節開始檢查，蛇頭不需要和自己比較。
    for (size_t i = 1; i < body.size(); i++) {
        if (body[i].equals(head)) {
            return true;
        }
    }

    return false;
}

bool Snake::occupies(Point p) const {
    // 判斷指定座標是否被任一節蛇身占用。
    for (const Point& part : body) {
        if (part.equals(p)) {
            return true;
        }
    }

    return false;
}

Food::Food() {
    // 使用系統提供的隨機種子初始化亂數引擎。
    random_device rd;
    randomEngine = mt19937(rd());
    position = { 1, 1 };
}

void Food::spawn(int width, int height, const Snake& snake) {
    // 食物只會出現在圍牆內側。
    uniform_int_distribution<int> xDist(1, width - 2);
    uniform_int_distribution<int> yDist(1, height - 2);

    // 若新位置和蛇身重疊，就重新產生座標。
    do {
        position.x = xDist(randomEngine);
        position.y = yDist(randomEngine);
    } while (snake.occupies(position));
}

Point Food::getPosition() const {
    return position;
}

Game::Game() : snake(width / 2, height / 2) {
    // 隱藏游標可避免主控台畫面更新時出現閃爍的游標。
    hideCursor();
    restart();
}

void Game::run() {
    draw();

    // 遊戲主迴圈：處理輸入、更新狀態、重新繪圖。
    while (true) {
        if (!gameOver) {
            if (!handleInput()) {
                break;
            }

            update();
            draw();

            // speed 越小，每次更新的間隔越短，遊戲速度越快。
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
    // 將所有會隨一局遊戲改變的狀態恢復成初始值。
    snake.reset(width / 2, height / 2);
    food.spawn(width, height, snake);

    gameOver = false;
    score = 0;

    speed = 230;
}

bool Game::handleInput() {
    // _kbhit() 可在不暫停遊戲迴圈的情況下檢查鍵盤輸入。
    if (_kbhit()) {
        int key = _getch();

        // 方向鍵會先回傳 224 或 0，第二次讀取才是實際按鍵碼。
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

    // 蛇頭碰到四周圍牆時結束遊戲。
    if (head.x <= 0 || head.x >= width - 1 || head.y <= 0 || head.y >= height - 1) {
        gameOver = true;
        return;
    }

    // 蛇頭和任一節蛇身重疊時結束遊戲。
    if (snake.hitSelf()) {
        gameOver = true;
        return;
    }

    // 吃到食物後增加分數、蛇身長度，並產生下一個食物。
    if (head.equals(food.getPosition())) {
        snake.grow();
        score += 10;
        food.spawn(width, height, snake);

        // 每得到 50 分加快一次，並設定速度下限。
        if (score % 50 == 0 && speed > 130) {
            speed -= 10;
        }
    }
}

void Game::draw() {
    setCursorToTopLeft();

    // 先在記憶體中組合完整畫面，再一次輸出以減少閃爍。
    vector<string> screen(height, string(width, ' '));

    // 繪製上下邊界。
    for (int x = 0; x < width; x++) {
        screen[0][x] = '#';
        screen[height - 1][x] = '#';
    }

    // 繪製左右邊界。
    for (int y = 0; y < height; y++) {
        screen[y][0] = '#';
        screen[y][width - 1] = '#';
    }

    // 使用 @ 表示食物。
    Point foodPos = food.getPosition();
    screen[foodPos.y][foodPos.x] = '@';

    const deque<Point>& body = snake.getBody();

    // 蛇頭使用 O，其他蛇身使用 o。
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

    // 將完成的畫面逐列輸出到主控台。
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
    // 每一幀都從主控台左上角覆寫，避免畫面不斷向下捲動。
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD position = { 0, 0 };
    SetConsoleCursorPosition(console, position);
}

void Game::hideCursor() {
    // 隱藏主控台文字游標，讓遊戲畫面更乾淨。
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(console, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(console, &cursorInfo);
}
