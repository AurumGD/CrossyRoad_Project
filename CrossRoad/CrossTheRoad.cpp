#include <SFML/Audio.hpp>
#include <iostream>  // For cout, endl, and cin
#include <string>    // For the 'string' data type in your Node struct
#include <ctime>     // For clock() to manage truck movement speed
#include <fstream>
#include <conio.h>
#include <ctime>
#include <limits>
#include <random>
#include <windows.h> // For Sleep() or console clearing (common in Windows dev)
using namespace std;

void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

struct Sound {
    string fileName;
    float localVolume; // 0.0 to 1.0
};

struct Node{
    string data;
    Node* next = nullptr;
};
typedef Node* NodePtr;

class ScoringSystem {
  private:
	int time = 240;
	int score = 0;
  public:

};

class Player {
  private:
	sf::SoundBuffer hopBuffer;
    sf::Sound hop;
    sf::Music collide;
  public:
	Player() : hop(hopBuffer) {}

	void loadSettings() {
        if (!hopBuffer.loadFromFile("sfx/hop.mp3")) {
            cout << "Error loading hop sound!" << endl;
        }
        hop.setBuffer(hopBuffer);

        if (!collide.openFromFile("music/defeat.mp3")) { // Use ->
            cout << "Error loading defeat music!" << endl;
        }
    }

	void playerMovement(int &x, int &y) {
		if (GetAsyncKeyState(VK_UP)) {
			if (y > 0) {
				y--;
				hop.play();
				Sleep(25);
			}
		}
		if (GetAsyncKeyState(VK_DOWN)) {
			if (y < 19) {
				y++;
				hop.play();
				Sleep(25);
			}
		}
		if (GetAsyncKeyState(VK_RIGHT)) {
			if (x < 39) {
				x++;
				hop.play();
			}
		}
		if (GetAsyncKeyState(VK_LEFT)) {
			if (x > 0) {
				x--;
				hop.play();
			}
		}
	}

	void defeat() {
        collide.play();
    }
	
	char getCharAtPos(NodePtr head, int x) {
	    NodePtr temp = head;
	    for (int i = 0; i < max(0, min(x, 39)); i++) temp = temp->next; // Walk to the player's X position
	    return temp->data[0]; // Return the char ('.' , '#', or '~')
	}
	
	bool collision(char symbol, int currentLane) {
		bool isLogLane = (currentLane >= 2 && currentLane <= 3) || (currentLane >= 11 && currentLane <= 12);
		bool isTruckLane = (currentLane >= 5 && currentLane <= 9) || (currentLane >= 15 && currentLane <= 18);
		
		if (isLogLane) {
			return (symbol != '~'); 

		}

		if (isTruckLane) {
			return (symbol != '#');

		}
		return true; 
	}
};

class Game {
  private:
    string pattern = "";
    int lane = 0;
	int playerX = 20, playerY = 19;
	NodePtr lanes[20];

    bool gameRunning = true;
    bool victory = false;
	
	clock_t lastMapUpdate = clock();
	int mapSpeed = 180;

	sf::Music myMusic;
	sf::Music resultMusic;
	sf::SoundBuffer countdownBuffer;
	sf::Sound countdown {countdownBuffer};

	void displayLane(NodePtr head) {
	    NodePtr temp = head;				// Start at the beginning
	    for (int i = 0; i < 40; i++) {
			cout << temp->data;				// Print the "." or "#"
		    temp = temp->next;				// Move to the next node in the chain
	    }
	}

	void initializeLane(NodePtr &head, string pattern) {
	    head = new Node;
	    head->data = string(1, pattern[0]);
	    NodePtr temp = head;
	
	    for (int i = 1; i < 40; i++) {
	        temp->next = new Node;
			temp = temp->next;
			temp->data = string(1, pattern[i]);
			temp->next = nullptr;
	    }
	    temp->next = head; 
	}
	
  public:
	Game(){
		for (int i = 0; i < 20; i++) {
			lanes[i] = nullptr;
		}
	}

	bool mark(bool gameRunning){
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		int startX = 1;
		int startY = 6; 
		
		if (!countdownBuffer.loadFromFile("sfx/countdown.mp3")) {
			cout << "Failed to load countdown music!" << endl;
		}
		countdown.play();
		
		string* sequence[4];
		
		string halt[] = {
			"========================================",
			"                                        ",
			"          H  H   &   H     TEAET        ",
			"          H  H  p q  I       H          ",
			"          ECCH  A=A  I       H          ",
			"          H  H I   I H       H          ",
			"          H  H H   H CEEEH   H          ",
			"                                        ",
			"========================================"
		};

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

		sequence[0] = halt;
		sequence[1] = ready;
        sequence[2] = set;
        sequence[3] = go;

        for (int stage = 0; stage < 4; stage++) {
			if (stage == 0) setColor(8);
            else if (stage < 3) setColor(14); 
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

	void finishLine() {
	    pattern = "";
	    for (int j = 0; j < 8; j++) pattern += "=~=~=";
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
				pattern = "=====~~~~~~~=====~~~~=====~~~~~~~~~=====";
			}
			initializeLane(lanes[lane], pattern);
			lane++;
		}
	}
	void truckRoad () {
		static mt19937 gen(static_cast<unsigned int>(time(0)));
		uniform_int_distribution<> dis(1, 8);
		for (int i = 0; i < 6; i++) { // Changed <= 5 to < 6
			if (lane >= 20) break; // Safety check

			if (i == 0) {
				pattern = "........................................"; // Exact 40
			} else {
				int choice = dis(gen);
				if (choice == 1)      pattern = "###.....#####..####...###.....#####...##";
				else if (choice == 2) pattern = "..###......######.........####.......##.";
				else if (choice == 3) pattern = "#####...######...#######...#######..####";
				else if (choice == 4) pattern = "..###....####.....####...##..#####......s";
				else if (choice == 5) pattern = "..##...##...##...##...##...##...##...##.";
				else if (choice == 6) pattern = ".....############......#########........";
				else if (choice == 7) pattern = "..........######...######......#####..##";
				else                  pattern = ".##....................####.....#####...";
				
				while(pattern.length() < 40) pattern += "."; 
				if(pattern.length() > 40) pattern = pattern.substr(0, 40);
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
		if (victory) {
			if (!resultMusic.openFromFile("music/victory.mp3")) {
				cout << "Failed to load victory music!" << endl;
			}
		}
		else {
			if (!resultMusic.openFromFile("music/defeat.mp3")) {
				cout << "Failed to load defeat music!" << endl;
			}
		}
		resultMusic.play();
		
		const string* finalArt;

		string winArt[] = {
			"========================================",
			"                                        ",
			" H   H THT .CEL THEHT  .==.  RER, V   V ",
			" H   H  I  C      H   A    A H  7  Y Y  ",
			" Y   Y  I  H      H   H    H BEr    V   ",
			"  V V   I  C      H   Y    Y H  T   I   ",
			"   V   JHL 'CCE   H    ^==^  H  T   I   ",
			"                                        ",
			"        ANGAS AF, BALIK ARAL NA...      ",
			"========================================"
		};

		string loseArt[] = {
			"========================================",
			"                                        ",
			"  DEEe. CEEET CEEEE CEEET    &   TEAET  ",
			"  E   H E     H     H       A A    H    ",
			"  E   H CEEr  CET   CEEr    q=b    H    ",
			"  E   H E     H     H      A   A   H    ",
			"  DEEE* CEEEL H     CEEEL  H   H   H    ",
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

		setColor(7);
		SetConsoleCursorPosition(hConsole, { 0, 22 });
	}

	void Layout() { //Editable
		finishLine(); // Adds 1 lane  (Total: 1)
		logRiver();   // Adds 3 lanes (Total: 4)
		truckRoad();  // Adds 6 lanes (Total: 10)
		logRiver();   // Adds 3 lanes (Total: 13)
		truckRoad();  // Adds 6 lanes (Total: 19)
		startLine();  // Adds 1 lane  (Total: 20)
		drawGameField(lanes, playerX, playerY, gameRunning, victory);
	}
	void GameRunning() {
		if (!myMusic.openFromFile("music/fun_farts.mp3")) {
			cout << "Error: Could not find the music file!" << endl;
		} else {
			myMusic.setLooping(true);
			myMusic.setVolume(100.f);
			myMusic.play();
		}

		Player name;
		name.loadSettings();
		mark(gameRunning);
    
		while (gameRunning) {
			drawGameField(lanes, playerX, playerY, gameRunning, victory);
			name.playerMovement(playerX, playerY);

			if (clock() - lastMapUpdate > mapSpeed) {
				for (int i = 0; i < 20; i++) {
					if (i == 0 || i == 19) continue;
						if (i % 2 == 0) { 
						lanes[i] = lanes[i]->next;
					} else {
						for (int step = 0; step < 39; step++) {
							lanes[i] = lanes[i]->next;
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

				char standingOn = name.getCharAtPos(lanes[playerY], playerX);
				if (!name.collision(standingOn, playerY)){
					name.defeat();
					gameRunning = false;
				}
				lastMapUpdate = clock();
			}

			drawGameField(lanes, playerX, playerY, gameRunning, victory);
			
			if (playerY == 0) {
				victory = true;
				gameRunning = false; 
			}
		}
		myMusic.stop();

		drawGameField(lanes, playerX, playerY, false, victory);
		drawResultBox(victory);

		cout << "\nPress Enter to Continue";
		cin.ignore(std::numeric_limits<streamsize>::max(),'\n');
	}
};

class Menu {
private:
	Game CrossRoad;
	sf::Music background_music;
	bool Music = false;
public:
	void Settings() {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		int startX = 1;
		int startY = 2; 

		string settings[] = {
			"<======================================>",
			"| .=. r-. .=. .=. .=.  r-. .=. .=. r=. |",
			"| l   t<' l l '=. '=.  t<' l l t-t l l |",
			"| '=^ l l '=' '=' '='  l l '=' l l t=' |",
			"<======================================>",
			"|  Settings:                           |",
			"|    Music: [##########] 100% (+/-)    |",
			"|    SFX  : [##########] 100% (+/-)    |",
			"|  1. Exit                             |",
			"<======================================>",
		};

		system("cls");
		for (int i = 0; i < 10; i++) {
            SetConsoleCursorPosition(hConsole, { (SHORT)startX, (SHORT)(startY + i) });
            cout << settings[i];
        }

		while (true) {
			char choice = _getch();
			if (choice == '1') break;
		}
	}
	void Credits() {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		int startX = 1;
		int startY = 2; 

		string credits[] = {
			"<======================================>",
			"| .=. r-. .=. .=. .=.  r-. .=. .=. r=. |",
			"| l   t<' l l '=. '=.  t<' l l t-t l l |",
			"| '=^ l l '=' '=' '='  l l '=' l l t=' |",
			"<======================================>",
			"|  Credits:              ..            |",
			"|    Albeus, Rex         i|  ;D        |",
			"|    Balondo, Renz       j|  u.u       |",
			"|    Cepeda, Nico        l|  q-q       |",
			"|    Sto.Domingo, Lander t|  O-O       |",
			"|  1. Exit               ''            |",
			"<======================================>"
		};

		system("cls");
		for (int i = 0; i < 12; i++) {
            SetConsoleCursorPosition(hConsole, { (SHORT)startX, (SHORT)(startY + i) });
            cout << credits[i];
        }

		while (true) {
			char choice = _getch();
			if (choice == '1') break;
		}
		
	}
	bool TitleScreen() {
		if (!background_music.openFromFile("music/wonder_run.mp3")) {
			cout << "Error: Could not find the music file!" << endl;
		}
		else {
			if (!Music) {
				background_music.setLooping(true);
				background_music.setVolume(100.f);
				background_music.play();
				Music = true;
			}
			else {
				background_music.setLooping(true);
				background_music.setVolume(100.f);
			}
		}
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		int startX = 1;
		int startY = 2; 

		Game CrossRoad;

		string title[] = {
			"<======================================>",
			"| .=. r-. .=. .=. .=.  r-. .=. .=. r=. |",
			"| l   t<' l l '=. '=.  t<' l l t-t l l |",
			"| '=^ l l '=' '=' '='  l l '=' l l t=' |",
			"<======================================>",
			"|  1. Play                             |",
			"|  2. Settings                         |",
			"|  3. Credits                          |",
			"|  4. Quit                             |",
			"<======================================>",
		};
		
		system("cls");

		for (int i = 0; i < 10; i++) {
            SetConsoleCursorPosition(hConsole, { (SHORT)startX, (SHORT)(startY + i) });
            cout << title[i];
        }

		char choice = _getch();
		if (choice == '1'){
			background_music.pause();
			Music = false;
			CrossRoad.Layout();
			CrossRoad.GameRunning();
		}
		if (choice == '2'){
			Settings();
		}
		if (choice == '3'){
			Credits();
		}
		if (choice == '4'){
			background_music.stop();
			return false;
		}
		return true;
	}
};

int main() {
    Menu Program;
    bool isRunning = true;
    while (isRunning) {
        isRunning = Program.TitleScreen();
    }
    return 0;
}
