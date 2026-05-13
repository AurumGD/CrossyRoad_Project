#include <iostream>  // For cout, endl, and cin
#include <string>    // For the 'string' data type in your Node struct
#include <ctime>     // For clock() to manage truck movement speed
#include <fstream>
#include <conio.h>
#include <windows.h> // For Sleep() or console clearing (common in Windows dev)
#include <SFML/Audio.hpp>

// --- NEW ADDITIONS FOR LEADERBOARD ---
#include <vector>
#include <algorithm>
// -------------------------------------

using namespace std;

void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

struct Node{
    string data;
    Node* next;
};
typedef Node* NodePtr;

class intro {
  public:
    bool mark(bool gameRunning){
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        int startX = 1;
        int startY = 6; 
        
        string* sequence[3];

        string ready[] = {
            "========================================",
            "                                        ",
            "       r=r. C===  &.  DEh.  V   V       ",
            "       H  H E     p q  H  H   V V       ",
            "       R=C  C===  A=A  H  I    Y        ",
            "       H  R E    I   I H  H    I        ",
            "       H  H C=== H   H DEE^    I        ",
            "                                        ",
            "========================================"
        };

        string set[] = {
            "========================================",
            "                                        ",
            "             .c== C=== ==v==            ",
            "             S    E      T              ",
            "              SES C===   I              ",
            "                H E      I              ",
            "             ===^ C===   H              ",
            "                                        ",
            "========================================"
        };

        string go[] = {
            "========================================",
            "                                        ",
            "             .c===.  .==.               ",
            "             G      A    A              ",
            "             G  4EG H    H              ",
            "             G    H Y    Y              ",
            "             ^====G  ^==^               ",
            "                                        ",
            "========================================"
        };

        sequence[0] = ready;
        sequence[1] = set;
        sequence[2] = go;

        for (int stage = 0; stage < 3; stage++) {
            if (stage < 2) setColor(14); 
            else setColor(10);

            for (int i = 0; i < 9; i++) {
                SetConsoleCursorPosition(hConsole, { (SHORT)startX, (SHORT)(startY + i) });
                cout << sequence[stage][i];
            }
            Sleep(1000); 
        }

        setColor(7);
        return true;
    }
};

class obstacle {
  private:
    string pattern = "";
    int lane = 0;
    
  public:
    NodePtr lanes[20];
    
    void displayLane(NodePtr head) {
        NodePtr temp = head;                // Start at the beginning
        for (int i = 0; i < 40; i++) {
            cout << temp->data;             // Print the "." or "#"
            temp = temp->next;              // Move to the next node in the chain
        }
    }
    
    char getCharAtPos(NodePtr head, int x) {
        NodePtr temp = head;
        for (int i = 0; i < x; i++) temp = temp->next; // Walk to the player's X position
        return temp->data[0]; // Return the char ('.' , '#', or '~')
    }
    
    bool collision(char symbol, int currentLane) {
        bool isLogLane = (currentLane >= 2 && currentLane <= 3) || (currentLane >= 12 && currentLane <= 13);
        bool isTruckLane = (currentLane >= 5 && currentLane <= 9) || (currentLane >= 15 && currentLane <= 18);
        
        if (isLogLane) {
            return (symbol != '~'); 
        }

        if (isTruckLane) {
            return (symbol != '#');
        }

        return true; 
    }
    
    void drawGameField(NodePtr lanes[], int pX, int pY, bool running, bool victory) {
        COORD cursorPosition;
        cursorPosition.X = 0;
        cursorPosition.Y = 0;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);

        setColor(7);
        cout << "==========================================\n"; // Top Border
        for (int i = 0; i < 20; i++) {
            setColor(7);
            cout << "|";
            NodePtr temp = lanes[i];
            for (int j = 0; j < 40; j++) {
                if (i == pY && j == pX) {
                    if (running || victory) {
                        setColor(10);
                        cout << "P";
                    } else {
                        setColor(12);
                        cout << "X";
                    }
                } else {
                    char symbol = temp->data[0];
                    switch (symbol) {
                        case '#': setColor(15); break; // Red for Trucks
                        case '.': setColor(8);  break; // Gray for Road/Grass
                        case '~': setColor(11);  break; // Blue for Water
                        case '=': setColor(4);  break; // Brown/Yellow for Logs
                        default:  setColor(7);  break; // Default White
                    }
                    cout << symbol;
                }
                temp = temp->next;   
            }
            setColor(7);
            cout << "|" << endl;
        }
        cout << "==========================================\n";
    }
    
    void initializeLane(NodePtr &head, string pattern) {
        head = new Node;
        head->data = string(1, pattern[0]);
        NodePtr temp = head;
    
        for (int i = 1; i < 40; i++) {
            temp->next = new Node;
            temp = temp->next;
            temp->data = string(1, pattern[i]);
        }
        temp->next = head; 
    }

    void finishLine() {
        pattern = "";
        for (int j = 0; j < 8; j++) pattern += "=~=~="; // Finish Line Pattern
        initializeLane(lanes[lane], pattern);
        lane++;
    }
    void logRiver () {
        for (int i = 0; i <= 2; i++) {
            pattern = "";
            if (i == 0) {
                for (int j = 0; j < 8; j++) pattern += ".....";
            }
            else {
                for (int j = 0; j < 4; j++) {
                    pattern += "=====";
                    pattern += "~~~~~";
                }
            }
            initializeLane(lanes[lane], pattern);
            lane++;
        }
    }
    void truckRoad () {
        for (int i = 0; i <= 5; i++) {
            pattern = "";
            if (i == 0) {
                for (int j = 0; j < 8; j++) pattern += ".....";
            }
            else {
                for (int j = 0; j < 4; j++) {
                    pattern += "#####";
                    pattern += ".....";
                }
            }
            initializeLane(lanes[lane], pattern);
            lane++;
        }
    }
    void startLine() {
        pattern = "";
        for (int j = 0; j < 8; j++) pattern += "....."; // Safe Start Zone
        initializeLane(lanes[lane], pattern);
        lane++;
    }

    void drawResultBox(bool victory) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        int startX = 1;
        int startY = 6; 
        
        const string* finalArt;

        string winArt[] = {
            "========================================",
            "                                        ",
            " V   V  H=H  7EP THE7T  y=t   H3R,  V  /",
            " H   H   I  C      H   7/  Y  E  7  Y v ",
            " v   y   I  E      I  //   T  LEC    Y  ",
            "  t y    I  C      I  H    /  I  L   I  ",
            "   V    H=H  VCE   I   Y=^    H  H   H  ",
            "                                        ",
            "        ANGAS AF, BALIK ARAL NA...      ",
            "========================================"
        };

        string loseArt[] = {
            "========================================",
            "                                        ",
            "V   /  y=t  II II   H      y=t  yo~ Ee==",
            " v v  7/ t  || ||   I     7  t H    E   ",
            "  Y  //  T  || ||   I    /   T  Yoy Ee==",
            "  H  H   /  II //   I    H   /    H E   ",
            "  I   Y=^   HI//    C===  YO^  ^suy Ee==",
            "                                        ",
            "        BETTER LUCK NEXT TIME...        ",
            "========================================"
        };

        if (victory) {
            setColor(10);
            finalArt = winArt;
        } else {
            setColor(12);
            finalArt = loseArt;
        }

        for (int i = 0; i < 10; i++) {
            SetConsoleCursorPosition(hConsole, { (SHORT)startX, (SHORT)(startY + i) });
            cout << finalArt[i];
        }

        SetConsoleCursorPosition(hConsole, { (SHORT)(startX + 16), (SHORT)(startY + 11) });
        setColor(15); // White
        cout << "Retry? (Y/N)";

        setColor(7);
        SetConsoleCursorPosition(hConsole, { 0, 22 });
    }
};

// --- NEW ADDITIONS: LEADERBOARD SYSTEM ---
struct PlayerScore {
    string name;
    int score;
};

// Helper function to sort scores from highest to lowest
bool compareScores(PlayerScore a, PlayerScore b) { 
    return a.score > b.score; 
}

class Leaderboard {
public:
    void saveScore(string name, int score) {
        vector<PlayerScore> scores;
        ifstream inFile("leaderboard.txt");
        string n; 
        int s;
        
        // Read existing scores
        while (inFile >> n >> s) {
            scores.push_back({n, s});
        }
        inFile.close();

        // Add new score and sort
        scores.push_back({name, score});
        sort(scores.begin(), scores.end(), compareScores);

        // Write the top 5 scores back to the file
        ofstream outFile("leaderboard.txt");
        for (int i = 0; i < min(5, (int)scores.size()); i++) { 
            outFile << scores[i].name << " " << scores[i].score << endl;
        }
        outFile.close();
    }

    void display() {
        setColor(14); // Yellow text
        cout << "\n\n================ TOP 5 LEADERBOARD ================\n";
        ifstream inFile("leaderboard.txt");
        string n; 
        int s;
        int rank = 1;
        while (inFile >> n >> s && rank <= 5) {
            cout << "  " << rank << ". " << n << " - " << s << " pts" << endl;
            rank++;
        }
        if (rank == 1) cout << "  No scores yet!\n";
        cout << "===================================================\n";
        setColor(7); // Reset to default
    }
};
// -----------------------------------------

int main() {
    sf::Music myMusic;
    obstacle gameMap;
    intro start;
    Leaderboard board;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if (!myMusic.openFromFile("music/fun_farts.mp3")) {
        cout << "Error: Could not find the music file!" << endl;
    } else {
        myMusic.setLooping(true);  
        myMusic.setVolume(100.f); 
        myMusic.play();          
    }

    // Set up the map layout once
    gameMap.finishLine(); 
    gameMap.logRiver();   
    gameMap.truckRoad();  
    gameMap.logRiver();   
    gameMap.truckRoad();  
    gameMap.startLine();  

    bool playAgain = true;

    // --- MASTER RESTART LOOP ---
    while (playAgain) {
        // Reset state for a fresh game
        int playerX = 20, playerY = 19;
        int lives = 3;
        int score = 0;
        int highestY = 19; 
        bool gameRunning = true;
        bool victory = false;
        
        system("cls"); // Clear screen for a fresh draw
        
        // The main Game Wrapper (Handles Lives)
        while (lives > 0 && !victory) {
            playerX = 20; 
            playerY = 19;
            highestY = 19;
            gameRunning = true;
            
            gameMap.drawGameField(gameMap.lanes, playerX, playerY, gameRunning, victory);
            start.mark(gameRunning);
            
            // Wipe memory of any keys pressed during the "Ready Set Go" intro
            for (int k = 0; k < 256; k++) GetAsyncKeyState(k);
            while (_kbhit()) _getch(); // Aggressive buffer wipe

            // NEW: Timer to control environmental speed separately from player speed
            int timer = 0; 

            while (gameRunning) {
                gameMap.drawGameField(gameMap.lanes, playerX, playerY, gameRunning, victory);
                
                SetConsoleCursorPosition(hConsole, {0, 22}); 
                setColor(11);
                cout << "LIVES: ";
                for (int l = 0; l < lives; l++) cout << "<3 "; 
                cout << "   |   SCORE: " << score << "        "; 
                setColor(7);

                // --- PLAYER MOVEMENT (Updates every 50ms for snappy controls) ---
                if ((GetAsyncKeyState(VK_UP) & 0x8000) && playerY > 0){
                    playerY--;
                    if (playerY < highestY) { 
                        score += 10;          
                        highestY = playerY;   
                    }
                }
                if ((GetAsyncKeyState(VK_DOWN) & 0x8000) && playerY < 19) playerY++;
                if ((GetAsyncKeyState(VK_LEFT) & 0x8000) && playerX > 0) playerX--;
                if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) && playerX < 39) playerX++;

                // --- OBSTACLES MOVEMENT (Updates only every 5 loops = 250ms) ---
                if (timer % 5 == 0) {
                    for (int i = 0; i < 20; i++) {
                        if (i == 0 || i == 19) continue;

                        if (i % 2 == 0) { 
                            gameMap.lanes[i] = gameMap.lanes[i]->next;
                        } else {
                            for (int step = 0; step < 39; step++) {
                                gameMap.lanes[i] = gameMap.lanes[i]->next;
                            }
                        }

                        bool isRiver = (i >= 2 && i <= 3) || (i >= 11 && i <= 12);
                        if (i == playerY && isRiver) {
                            if (i % 2 == 0) {
                                if (playerX < 39) playerX--; 
                                else gameRunning = false;
                            } else {
                                if (playerX > 0) playerX++; 
                                else gameRunning = false;
                            }
                        }
                    }
                }

                char standingOn = gameMap.getCharAtPos(gameMap.lanes[playerY], playerX);
                if (gameMap.collision(standingOn, playerY) == false) gameRunning = false;

                Sleep(50); // FAST loop for instant input response
                timer++;   // Increase the timer

                if (playerY == 0) {
                    victory = true;
                    score += 100; 
                    gameRunning = false; 
                }
            }
            
            // Death Logic
            if (!victory) {
                lives--;
                if (lives > 0) {
                    SetConsoleCursorPosition(hConsole, {0, 22});
                    setColor(12);
                    cout << "Ouch! You got hit! Restarting level...        ";
                    Sleep(2000); 
                    
                    // Erase the "Ouch" message so it doesn't leave ghosts on the screen
                    SetConsoleCursorPosition(hConsole, {0, 22});
                    cout << "                                              ";
                }
            }
        } // End of outer 'lives' loop

        gameMap.drawGameField(gameMap.lanes, playerX, playerY, false, victory);
        gameMap.drawResultBox(victory);

        // --- FIX: Safely wait for user to press Y or N ---
        while (_kbhit()) _getch(); // Destroy leftover arrow keys before asking
        
        while (true) {
            if (_kbhit()) {
                char choice = toupper(_getch()); // reads key and makes it uppercase
                if (choice == 'Y') {
                    playAgain = true;
                    break;
                } else if (choice == 'N') {
                    playAgain = false;
                    break;
                }
            }
            Sleep(50);
        }

        // --- Leaderboard Phase ---
        system("cls"); // Clear map out of the way for Leaderboard
        SetConsoleCursorPosition(hConsole, {0, 0});
        
        cout << "\nGame Over! You finished with a Score of: " << score << endl;
        cout << "Enter your Name for the Leaderboard (No Spaces): ";
        
        string pName;
        if (cin >> pName) {
            board.saveScore(pName, score);
            board.display();
        }

        // --- CRITICAL FIX: DESTROY THE ENTER KEY ---
        // This clears standard C++ inputs so they don't break the _kbhit pause
        cin.clear();
        cin.ignore(10000, '\n');
        
        // Final aggressive console sweep just in case
        while (_kbhit()) _getch();
        
        if (playAgain) {
            cout << "\nPress any key to restart game...";
            while (!_kbhit()); // Correctly wait for a new keypress
            _getch();          // Consume it
        }
    } 
    // --- END OF MASTER RESTART LOOP ---

    while (_kbhit()) _getch(); // Final sweep before exit
    cout << "\nThanks for playing! Press any key to exit...";
    while (!_kbhit());
    
    return 0;
}
