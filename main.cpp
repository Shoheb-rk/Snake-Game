
#include <iostream>
#include <conio.h>
#include <windows.h> // For Sleep(), Colors, and gotoxy()
#include <vector>
#include <string>
#include <fstream>
#include <ctime>
#include <cstdlib>

using namespace std;

// 1. Enumeration for Movement and UI Scenes
enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
enum Scene { MENU, PLAYING, GAMEOVER };

// 2. Templates (Generic function for High Score)
template <class T>
T findMax(T a, T b) {
    if (a > b) return a;
    return b;
}

// 3. Structures, Constructors, and Operator Overloading
struct Point {
    int x;
    int y;
    
    Point() { x = 0; y = 0; }
    Point(int a, int b) { x = a; y = b; }
    
    // Operator overloading to check tail collisions
    bool operator==(const Point& other) {
        return (x == other.x && y == other.y);
    }
};

// 4. Inheritance and Virtual Functions (Polymorphism)
class GameEntity {
protected:
    int x, y;
public:
    GameEntity() { x = 0; y = 0; }
    virtual void setPosition(int newX, int newY) {
        x = newX;
        y = newY;
    }
    int getX() { return x; }
    int getY() { return y; }
    
    // Pure virtual function
    virtual WORD getColor() = 0; 
};

// Derived Class
class Food : public GameEntity {
public:
    WORD getColor() {
        return BACKGROUND_RED | BACKGROUND_INTENSITY; // Red color for food
    }
};

// Classic beginner function to move the console cursor (Stops screen flickering!)
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// 5. Classes, Objects, Vectors, File Handling
class SnakeGame {
private:
    Scene currentScene;
    bool exitRequested;
    bool paused;
    int menuSelection;
    int gameOverSelection;

    const int width = 20;
    const int height = 20;

    int score;
    int highScore;

    Point head;
    Food fruit;
    
    // Parallel vectors instead of complex linked lists
    vector<int> tailX;
    vector<int> tailY;
    int nTail;
    Direction dir;

    HANDLE console;

public:
    SnakeGame() {
        console = GetStdHandle(STD_OUTPUT_HANDLE);
        
        // Hide the blinking cursor
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(console, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(console, &cursorInfo);

        exitRequested = false;
        currentScene = MENU;
        menuSelection = 0;
        gameOverSelection = 0;
        highScore = 0;

        // Simple File I/O without Exception Handling
        ifstream file("highscore.txt");
        if (file.is_open()) {
            file >> highScore;
            file.close();
        }

        resetGame();
    }

    ~SnakeGame() {
        // Save highest score when game closes
        ofstream file("highscore.txt");
        file << findMax(score, highScore);
        file.close();
    }

    void resetGame() {
        dir = STOP;
        head.x = width / 2;
        head.y = height / 2;
        srand(time(0));
        fruit.setPosition(rand() % (width - 2) + 1, rand() % (height - 2) + 1);
        score = 0;
        nTail = 0;
        tailX.clear();
        tailY.clear();
        paused = false;
        system("cls"); // Clear the screen cleanly on reset
    }

    bool isExitRequested() { return exitRequested; }

    // Helper to print colored text/blocks
    void printColor(string text, WORD color) {
        SetConsoleTextAttribute(console, color);
        cout << text;
        // Reset back to standard white text on black background
        SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    void Draw() {
        gotoxy(0, 0); // Start drawing from the top left every frame

        if (currentScene == MENU) {
            DrawMenu();
        }
        else if (currentScene == PLAYING) {
            DrawPlaying();
        }
        else if (currentScene == GAMEOVER) {
            DrawGameOver();
        }
    }

    void DrawMenu() {
        // Simple manual spacing to center things
        cout << "\n\n\n\n\n";
        cout << "               ";
        printColor("SNAKE GAME\n", FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        cout << "             High Score: " << highScore << "       \n\n\n";

        if (menuSelection == 0) {
            cout << "               ";
            printColor("[ START ]", BACKGROUND_BLUE | BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            cout << "   \n\n";
        } else {
            cout << "               [ START ]   \n\n";
        }

        if (menuSelection == 1) {
            cout << "               ";
            printColor("[ QUIT ]", BACKGROUND_BLUE | BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            cout << "   \n\n";
        } else {
            cout << "               [ QUIT ]   \n\n";
        }

        cout << "\n       Use Arrow Keys + Enter to select           \n";
        cout << "                                                  \n";
    }

    void DrawBoard() {
        for (int y = 0; y < height; y++) {
            cout << "          "; // Indent the board
            for (int x = 0; x < width; x++) {
                // Draw Walls (Cyan)
                if (x == 0 || x == width - 1 || y == 0 || y == height - 1) {
                    printColor("  ", BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY);
                }
                // Draw Snake Head (Green)
                else if (x == head.x && y == head.y) {
                    printColor("  ", BACKGROUND_GREEN | BACKGROUND_INTENSITY);
                }
                // Draw Food (Red) using our virtual function
                else if (x == fruit.getX() && y == fruit.getY()) {
                    printColor("  ", fruit.getColor());
                }
                else {
                    // Draw Tail
                    bool isTail = false;
                    for (int k = 0; k < nTail; k++) {
                        if (tailX[k] == x && tailY[k] == y) {
                            printColor("  ", BACKGROUND_GREEN | BACKGROUND_INTENSITY);
                            isTail = true;
                            break;
                        }
                    }
                    if (!isTail) {
                        cout << "  "; // Empty space inside the box
                    }
                }
            }
            cout << "          \n";
        }
        cout << "          Score: " << score << "   High Score: " << findMax(score, highScore) << "          \n";
        cout << "          Arrow Keys: Move   Space: Pause   Esc: Quit          \n";
    }

    void DrawPlaying() {
        DrawBoard();
        if (paused) {
            cout << "          ";
            printColor("PAUSED", FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            cout << "                                  \n";
        } else {
            cout << "                                                    \n";
        }
    }

    void DrawGameOver() {
        DrawBoard(); // Draw the board normally first

        // Now manually move the cursor to overwrite the center with the menu
        int textY = 6;
        
        gotoxy(19, textY);
        printColor("GAME OVER", FOREGROUND_RED | FOREGROUND_INTENSITY);

        gotoxy(18, textY + 2);
        if (gameOverSelection == 0) printColor("[ RETRY ]", BACKGROUND_BLUE | BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        else cout << "[ RETRY ]";

        gotoxy(16, textY + 4);
        if (gameOverSelection == 1) printColor("[ MAIN MENU ]", BACKGROUND_BLUE | BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        else cout << "[ MAIN MENU ]";

        gotoxy(18, textY + 6);
        if (gameOverSelection == 2) printColor("[ QUIT ]", BACKGROUND_BLUE | BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        else cout << "[ QUIT ]";

        gotoxy(8, textY + 8);
        cout << "Use Arrow Keys + Enter to select";
        
        // Move cursor down so it doesn't mess up our drawing
        gotoxy(0, height + 3); 
    }

    void Input() {
        if (_kbhit()) {
            int ch = _getch();
            if (ch == 0 || ch == 224) { 
                ch = _getch(); // Catch special arrow key codes
            }

            if (currentScene == MENU) {
                if (ch == 72) { // Up
                    menuSelection--;
                    if (menuSelection < 0) menuSelection = 1;
                }
                else if (ch == 80) { // Down
                    menuSelection++;
                    if (menuSelection > 1) menuSelection = 0;
                }
                else if (ch == 13) { // Enter
                    if (menuSelection == 0) {
                        resetGame();
                        currentScene = PLAYING;
                    } else {
                        exitRequested = true;
                    }
                }
                else if (ch == 27) { // Escape
                    exitRequested = true;
                }
            }
            else if (currentScene == GAMEOVER) {
                if (ch == 72) { // Up
                    gameOverSelection--;
                    if (gameOverSelection < 0) gameOverSelection = 2;
                }
                else if (ch == 80) { // Down
                    gameOverSelection++;
                    if (gameOverSelection > 2) gameOverSelection = 0;
                }
                else if (ch == 13) { // Enter
                    if (gameOverSelection == 0) {
                        resetGame();
                        currentScene = PLAYING;
                    }
                    else if (gameOverSelection == 1) {
                        system("cls");
                        currentScene = MENU;
                    }
                    else {
                        exitRequested = true;
                    }
                }
            }
            else if (currentScene == PLAYING) {
                switch (ch) {
                case 72: if (dir != DOWN) dir = UP; break;
                case 80: if (dir != UP) dir = DOWN; break;
                case 75: if (dir != RIGHT) dir = LEFT; break;
                case 77: if (dir != LEFT) dir = RIGHT; break;
                case 32: paused = !paused; break; // Spacebar
                case 27: exitRequested = true; break; // Esc
                }
            }
        }
    }

    void Logic() {
        if (currentScene != PLAYING || paused) return;

        // Vector shifting for the tail (Rookie style array shifting)
        int prevX = tailX.empty() ? 0 : tailX[0];
        int prevY = tailY.empty() ? 0 : tailY[0];
        int prev2X, prev2Y;

        if (nTail > 0) {
            tailX[0] = head.x;
            tailY[0] = head.y;
        }

        for (int i = 1; i < nTail; i++) {
            prev2X = tailX[i];
            prev2Y = tailY[i];
            tailX[i] = prevX;
            tailY[i] = prevY;
            prevX = prev2X;
            prevY = prev2Y;
        }

        switch (dir) {
        case LEFT:  head.x--; break;
        case RIGHT: head.x++; break;
        case UP:    head.y--; break;
        case DOWN:  head.y++; break;
        case STOP:  return; 
        }

        // Check Wall collision
        if (head.x >= width - 1 || head.x <= 0 || head.y >= height - 1 || head.y <= 0) {
            currentScene = GAMEOVER;
        }

        // Check Tail collision using our Point == Operator Overload
        for (int i = 0; i < nTail; i++) {
            Point currentTail(tailX[i], tailY[i]);
            if (head == currentTail) {
                currentScene = GAMEOVER;
            }
        }

        // Check Food collision
        if (head.x == fruit.getX() && head.y == fruit.getY()) {
            score += 10;
            nTail++;
            tailX.push_back(0); // Vector expands automatically
            tailY.push_back(0);

            // Re-spawn fruit
            bool validPosition = false;
            while (!validPosition) {
                fruit.setPosition(rand() % (width - 2) + 1, rand() % (height - 2) + 1);
                validPosition = true;
                
                for (int i = 0; i < nTail; i++) {
                    if (fruit.getX() == tailX[i] && fruit.getY() == tailY[i]) {
                        validPosition = false;
                    }
                }
                if (fruit.getX() == head.x && fruit.getY() == head.y) {
                    validPosition = false;
                }
            }
        }

        if (score > highScore) {
            highScore = score;
        }
    }
};

int main() {
    SnakeGame game;

    while (!game.isExitRequested()) {
        game.Draw();
        game.Input();
        game.Logic();
        Sleep(80); // Game loop speed
    }

    system("cls");
    return 0;
}