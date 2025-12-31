#include <iostream>
#include <cstdlib>              // for rand() and srand()
#include <time.h>
#include <SFML/Graphics.hpp>    // included SFML library
#include <fstream>              // for file handling
#include <cassert>              // for assert()

using namespace sf;             // required to use SFML library
using namespace std;

// Structure to hold player statistics
struct Player
{
    string name;
    int gamesPlayed = 0;
    int gamesWon = 0;
};

// Function to save player statistics into a file
void saveStats(Player p)
{
    ofstream fout("stats.txt"); // Opens file to WRITE
    assert(fout.is_open());     // Ensure file is open

    fout << p.name << endl;
    fout << p.gamesPlayed << endl;
    fout << p.gamesWon << endl;

    fout.close();               // Close the file
}

// Function to load player statistics from a file
Player loadStats()
{
    Player p;
    ifstream fin("stats.txt"); // Opens file to READ

    if (fin.is_open())
    {
        getline(fin, p.name);
        fin >> p.gamesPlayed;
        fin >> p.gamesWon;
        fin.close();
    }
    else
    {
        // Default values if file does not exist
        p.name = "Guest";
        p.gamesPlayed = 0;
        p.gamesWon = 0;
    }
    return p;
}

// Function to open empty cells and their surrounding cells (recursive logic)
void openEmpty(int x, int y, int grid[12][12], int sgrid[12][12])
{
    // Boundary check
    if (x < 1 || x > 10 || y < 1 || y > 10)
        return;

    // If already opened or flagged, stop
    if (sgrid[x][y] != 10)
        return;

    // Reveal the current cell
    sgrid[x][y] = grid[x][y];

    // If the cell is empty, open surrounding cells
    if (grid[x][y] == 0)
    {
        openEmpty(x + 1, y, grid, sgrid);
        openEmpty(x - 1, y, grid, sgrid);
        openEmpty(x, y + 1, grid, sgrid);
        openEmpty(x, y - 1, grid, sgrid);
        openEmpty(x + 1, y + 1, grid, sgrid);
        openEmpty(x - 1, y - 1, grid, sgrid);
        openEmpty(x + 1, y - 1, grid, sgrid);
        openEmpty(x - 1, y + 1, grid, sgrid);
    }
}

// Function that contains ONLY the SFML game logic
void runGame()
{
    const int gridSize = 12;     // 12 x 12 grid (border safety)
    const int tileSize = 32;     // size of one tile in pixels
    const int windowSize = 10 * tileSize;

    RenderWindow game(VideoMode(windowSize, windowSize), "SFML Minesweeper");

    Texture tileTexture;         // Texture object
    if (!tileTexture.loadFromFile("tiles.jpg"))
    {
        cout << "Error loading tiles.jpg" << endl;
        system("pause");
        return;
    }

    Sprite tile(tileTexture);    // Sprite object to draw tiles

    int grid[gridSize][gridSize] = {0};   // Actual game data
    int sgrid[gridSize][gridSize] = {0};  // What player sees

    // Randomly place mines and hide cells
    for (int i = 1; i <= 10; i++)
        for (int j = 1; j <= 10; j++)
        {
            sgrid[i][j] = 10;             // Hidden tile
            grid[i][j] = (rand() % 5 == 0) ? 9 : 0; // 20% mine chance
        }

    // Calculate numbers for non-mine cells
    for (int i = 1; i <= 10; i++)
        for (int j = 1; j <= 10; j++)
        {
            if (grid[i][j] == 9)
                continue;

            int count = 0;
            for (int dx = -1; dx <= 1; dx++)
                for (int dy = -1; dy <= 1; dy++)
                    if (grid[i + dx][j + dy] == 9)
                        count++;

            grid[i][j] = count;
        }

    bool gameOver = false; // Flag to stop game when mine is clicked

    // Main SFML Game Loop
    while (game.isOpen())
    {
        Event event;
        while (game.pollEvent(event))
        {
            // Close window
            if (event.type == Event::Closed)
                game.close();

            // Mouse click handling
            if (event.type == Event::MouseButtonPressed && !gameOver)
            {
                int x = event.mouseButton.x / tileSize;
                int y = event.mouseButton.y / tileSize;

                // Stay inside grid
                if (x >= 0 && x < 10 && y >= 0 && y < 10)
                {
                    int gx = x + 1;
                    int gy = y + 1;

                    // Left click = reveal
                    if (event.mouseButton.button == Mouse::Left)
                    {
                        if (grid[gx][gy] == 9)
                        {
                            gameOver = true;

                            // Reveal entire grid
                            for (int i = 1; i <= 10; i++)
                                for (int j = 1; j <= 10; j++)
                                    sgrid[i][j] = grid[i][j];

                            cout << "BOOM! Game Over!" << endl;
                        }
                        else
                        {
                            openEmpty(gx, gy, grid, sgrid);
                        }
                    }

                    // Right click = flag
                    if (event.mouseButton.button == Mouse::Right)
                    {
                        if (sgrid[gx][gy] == 10)
                            sgrid[gx][gy] = 11;
                        else if (sgrid[gx][gy] == 11)
                            sgrid[gx][gy] = 10;
                    }
                }
            }
        }

        game.clear(Color::White);

        // Draw the grid
        for (int i = 1; i <= 10; i++)
            for (int j = 1; j <= 10; j++)
            {
                tile.setTextureRect(
                    IntRect(sgrid[i][j] * tileSize, 0, tileSize, tileSize));
                tile.setPosition((i - 1) * tileSize, (j - 1) * tileSize);
                game.draw(tile);
            }

        game.display();
    }
}

int main()
{
    srand(time(0));                    // Seed random generator
    Player currentPlayer = loadStats(); // Load player stats

    // TERMINAL MENU LOOP
    while (true)
    {
        system("cls");
        cout << "===========================\n";
        cout << "   MINESWEEPER ULTIMATE\n";
        cout << "===========================\n";
        cout << "Welcome back, " << currentPlayer.name << "!\n\n";
        cout << "1. Play Game\n";
        cout << "2. View Stats\n";
        cout << "3. Change Name\n";
        cout << "4. Exit\n";
        cout << "Choose option: ";

        int choice;
        cin >> choice;

        if (choice == 1)
        {
            currentPlayer.gamesPlayed++;
            saveStats(currentPlayer);
            runGame();                 // Run SFML game
        }
        else if (choice == 2)
        {
            cout << "\n--- PLAYER STATS ---\n";
            cout << "Name: " << currentPlayer.name << endl;
            cout << "Played: " << currentPlayer.gamesPlayed << endl;
            cout << "Won: " << currentPlayer.gamesWon << endl;
            system("pause");
        }
        else if (choice == 3)
        {
            cout << "Enter new name: ";
            cin >> currentPlayer.name;
            saveStats(currentPlayer);
        }
        else if (choice == 4)
        {
            break;                     // Exit program
        }
    }

    return 0;
}
