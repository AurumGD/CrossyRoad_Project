#include <iostream>  // For cout, endl, and cin
#include <string>    // For the 'string' data type in your Node struct
#include <ctime>     // For clock() to manage truck movement speed
#include <fstream>
#include <conio.h>
#include <windows.h> // For Sleep() or console clearing (common in Windows dev)
#include <SFML/Audio.hpp>

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
			"       r=r. C===   &.  DEh.  V   V      ",
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
	    NodePtr temp = head;				// Start at the beginning
	    for (int i = 0; i < 40; i++) {
			cout << temp->data;				// Print the "." or "#"
		    temp = temp->next;				// Move to the next node in the chain
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

int main() {
    int playerX = 20, playerY = 19;
    bool gameRunning = true;
    bool victory = false;
	sf::Music myMusic;
    
    obstacle gameMap;
	intro start;

	if (!myMusic.openFromFile("music/fun_farts.mp3")) {
        cout << "Error: Could not find the music file!" << endl;
    }
	else {
        myMusic.setLooping(true);  // Make it repeat
        myMusic.setVolume(100.f); // Set volume (0.0 to 100.0)
        myMusic.play();          // Start the music
    }

    gameMap.finishLine(); // Adds 1 lane  (Total: 1)
	gameMap.logRiver();   // Adds 3 lanes (Total: 4)
	gameMap.truckRoad();  // Adds 6 lanes (Total: 10)
	gameMap.logRiver();   // Adds 3 lanes (Total: 13)
	gameMap.truckRoad();  // Adds 6 lanes (Total: 19)
	gameMap.startLine();  // Adds 1 lane  (Total: 20)
	gameMap.drawGameField(gameMap.lanes, playerX, playerY, gameRunning, victory);

	start.mark(gameRunning);
    
    while (gameRunning) {
        gameMap.drawGameField(gameMap.lanes, playerX, playerY, gameRunning, victory);
        
        if (GetAsyncKeyState(VK_UP) && playerY > 0){
			playerY--;
		}
        if (GetAsyncKeyState(VK_DOWN) && playerY < 19){
			playerY++;
		}
        if (GetAsyncKeyState(VK_LEFT) && playerX > 0){
			playerX--;
		}
        if (GetAsyncKeyState(VK_RIGHT) && playerX < 39){
			playerX++;
		}

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

        char standingOn = gameMap.getCharAtPos(gameMap.lanes[playerY], playerX);
		if (gameMap.collision(standingOn, playerY) == false) gameRunning = false;

        Sleep(250);
        
        if (playerY == 0) {
            victory = true;
            gameRunning = false; 
        }
    }
    gameMap.drawGameField(gameMap.lanes, playerX, playerY, false, victory);
	gameMap.drawResultBox(victory);

    Sleep(500); 
    cout << "\n\nPress any key to exit...";
    while (!_kbhit());
    
    return 0;
}