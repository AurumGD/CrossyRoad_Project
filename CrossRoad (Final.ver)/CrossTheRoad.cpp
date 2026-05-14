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
#include <cctype> // Essential for character checks

#include <vector>
#include <algorithm>

using namespace std;

void pressEnterToContinue() {
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

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
		if (GetAsyncKeyState(VK_UP) & 0x8000) {
			if (y > 0) {
				y--;
				hop.play();
			}
		}
		if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
			if (y < 19) {
				y++;
				hop.play();
			}
		}
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
			if (x < 39) {
				x++;
				hop.play();
			}
		}
		if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
			if (x > 0) {
				x--;
				hop.play();
			}
		}
		Sleep(6);
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
		bool isTruckLane = (currentLane >= 5 && currentLane <= 9) || (currentLane >= 14 && currentLane <= 18);
		
		if (isLogLane) {
			return (symbol != '~'); 
		}

		if (isTruckLane) {
			return (symbol != '#');
		}
		return true; 
	}
};

struct PlayerScore {
    string name;
    int score;
};

// Helper function to sort scores from highest to lowest
bool compareScores(PlayerScore a, PlayerScore b) { 
    return a.score > b.score; 
}

class Leaderboard {
private:
	string playerName;
	sf::Music musicResult;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
public:
	void nameOfPlayers(int score){
		if (!musicResult.openFromFile("music/oh_well.mp3")) {
			cout << "Failed to load results music!" << endl;
		}

		Sleep(3000);

		musicResult.setLooping(true);
		musicResult.setVolume(100.f);
		musicResult.play();
		
		SetConsoleCursorPosition(hConsole, {0, 22});
		setColor(14);
		cout << endl << endl
			 << "\n  .=. r. r- .=. ===  .=. .=. r.q r-  |||"
			 << "\n  !_l lt t: l-l  l   !_l l=l l'l t=  ..."
			 << "\n\n  Please Enter your name: ";

		while (true){
			cin.clear();
			getline(cin, playerName);
			bool hasNumber = false;
			bool hasSymbol = false;
			if (playerName.empty()) {
				cout << "  Name cannot be empty!\n  Try again: ";
				continue;
			}
			if (playerName.size() > 6) {
				cout << "  Name too long!\n  Try again: ";
				continue;
			}
			for (char c : playerName) {
				if (isdigit(c)) {
					hasNumber = true;
				} 
				else if (ispunct(c)) {
					hasSymbol = true;
				}
			}
			if (hasNumber || hasSymbol) cout << "  Invalid name!\n  Please input another name!: "; 
			else {
				system("cls");
				saveScore(playerName, score);
				display();
				break;
			}
		}
	}
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
		inFile.close();
        cout << "===================================================\n";
        setColor(7);
		Sleep(5000);
		musicResult.stop();
    }
};

class Game {
  private:
    string pattern = "";
    int lane = 0;
	int playerX = 20, playerY = 19;
	int lives = 3;
	int score = 0;
    int highestY = 19;
	bool collided = false;
	NodePtr lanes[20];

	Player name;
	Leaderboard players;

    bool gameRunning = true;
    bool victory = false;
	bool playing = true;
	
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	clock_t lastMapUpdate = clock();
	clock_t lastSecondUpdate = clock();
	int mapSpeed = 180;

	sf::Music myMusic;
	sf::Music resultMusic;
	sf::Music death;
	sf::SoundBuffer countdownBuffer;
	sf::Sound countdown {countdownBuffer};

	void displayLane(NodePtr head) {
	    NodePtr temp = head;
	    for (int i = 0; i < 40; i++) {
			cout << temp->data;
		    temp = temp->next;
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

	void clearLanes() {
		for (int i = 0; i < 20; i++) {
			if (lanes[i] == nullptr) continue;

			NodePtr current = lanes[i]->next;

			while (current != lanes[i]) {
				NodePtr temp = current;
				current = current->next;
				delete temp;
			}

			delete lanes[i];
			lanes[i] = nullptr;
		}
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
	void drawGameField(NodePtr lanes[], int pX, int pY, bool running, bool victory, bool collided) {
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
					if (running && collided) {
						setColor(12);
						cout << "X";
					}
					else if (running || victory) {
						setColor(10);
						cout << "P";
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
	    cout << "==========================================";
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

	void scoreLayout(int lives, int score, int &time){
		setColor(15);
		SetConsoleCursorPosition(hConsole, {0, 22}); 
		cout << "|";

		SetConsoleCursorPosition(hConsole, {1, 22}); 
        setColor(11);
        cout << " LIVES: ";
        for (int l = 0; l < lives; l++) cout << " <3 "; 

		setColor(15);
		SetConsoleCursorPosition(hConsole, {0, 23}); 
		cout << "|";

		SetConsoleCursorPosition(hConsole, {25, 22}); 
        setColor(15);
		cout << "|";

		SetConsoleCursorPosition(hConsole, {26, 22}); 
        setColor(15);
		cout << "SCORE: ";
		SetConsoleCursorPosition(hConsole, {34, 22});
		cout << score;

		SetConsoleCursorPosition(hConsole, {41, 22}); 
		setColor(15);
		cout << "|";

		SetConsoleCursorPosition(hConsole, {25, 23}); 
        setColor(15);
		cout << "|";

		SetConsoleCursorPosition(hConsole, {26, 23}); 
		setColor(15);
		cout << "TIME: " << time;

		SetConsoleCursorPosition(hConsole, {41, 23}); 
		setColor(15);
		cout << "|";

		SetConsoleCursorPosition(hConsole, {0, 24});
		setColor(15);
		cout << "==========================================";
        setColor(7);
	}

	void scoreSystem(int y, int &highestY, int &score) {
		if (y < highestY) {
			score += 10;
			highestY = y;
		}
	}

	void loseLife() {
		lives--;
		playerX = 20;
		playerY = 19;
		highestY = 19;

		if (lives > 0) {
			SetConsoleCursorPosition(hConsole, {0, 22}); 
        	setColor(15);
			cout << "|";

			SetConsoleCursorPosition(hConsole, {1, 22}); 
        	setColor(11);
        	cout << " LIVES: ";
        	for (int l = 0; l < lives; l++) cout << " <3 "; 
			setColor(12);
			cout << " </3 ";
			Sleep(2000);
		}
		else {
			myMusic.stop();
			drawResultBox(victory);
			setColor(15);
			cout << "| ";
			SetConsoleCursorPosition(hConsole, {2, 22});
			setColor(12);
			cout << "      GAME OVER!      ";
			players.nameOfPlayers(score);
		}
		collided = false;
		setColor(7);
	}
	void Layout() { //Editable
		lane = 0;
		clearLanes();
		finishLine(); // Adds 1 lane  (Total: 1)
		logRiver();   // Adds 3 lanes (Total: 4)
		truckRoad();  // Adds 6 lanes (Total: 10)
		logRiver();   // Adds 3 lanes (Total: 13)
		truckRoad();  // Adds 6 lanes (Total: 19)
		startLine();  // Adds 1 lane  (Total: 20)
		drawGameField(lanes, playerX, playerY, gameRunning, victory, collided);
	}
	void GameRunning() {
		if (!myMusic.openFromFile("music/fun_farts.mp3")) {
			cout << "Error: Could not find the music file!" << endl;
		}
		else {
			myMusic.setLooping(true);
			myMusic.setVolume(100.f);
			myMusic.play();
		}
		if (!death.openFromFile("music/defeat.mp3")) {
			cout << "Failed to load defeat music!" << endl;
		}

		playerX = 20;
		playerY = 19;
		lives = 3;
		score = 0;
		highestY = 19;

		playing = true;
		bool victory = false;
		bool collided = false;
		bool gameRunning = true;

		int time = 300;

		drawGameField(lanes, playerX, playerY, playing, victory, collided);
		scoreLayout(lives, score, time);
		
		name.loadSettings();
		mark(playing);

		while (playing && time > 0) {
			drawGameField(lanes, playerX, playerY, playing, victory, collided);
			scoreLayout(lives, score, time);
			
			name.playerMovement(playerX, playerY);
			scoreSystem(playerY, highestY, score);

			if (clock() - lastMapUpdate > mapSpeed) {
				for (int i = 0; i < 20; i++) {
					if (i == 0 || i == 19) continue;
					if (i % 2 == 0) {
						lanes[i] = lanes[i]->next;
					}
					else {
						for (int step = 0; step < 39; step++) {
							lanes[i] = lanes[i]->next;
						}
					}
					
					bool isRiver = (i >= 2 && i <= 3) || (i >= 11 && i <= 12);
					if (i == playerY && isRiver) {
						if (i % 2 == 0) {
							if (playerX < 39)
							playerX--;
							else gameRunning = false;
						}
						else {
							if (playerX > 0) playerX++;
							else gameRunning = false; }
						}
					}

				char standingOn = name.getCharAtPos(lanes[playerY], playerX);
				if (!name.collision(standingOn, playerY)) {
					collided = true;
					drawGameField(lanes, playerX, playerY, playing, victory, collided);
					death.play();
					loseLife();
					if (lives == 0) {
						playing = false;
						break;
					}
					collided = false;
				}
				lastMapUpdate = clock();
			}

			drawGameField(lanes, playerX, playerY, playing, victory, collided);
			if (playerY == 0) {
				victory = true;
				score += 200;
				playing = false;
			}
			if (clock() - lastSecondUpdate >= CLOCKS_PER_SEC) {
				if (score > 0) score--;
				time--;
				lastSecondUpdate = clock();
			}
		}
		if (victory == true){
			myMusic.stop();
			drawResultBox(victory);
			for (int i = 0; i < lives; i++) score += 300;
			scoreLayout(lives, score, time);
			Sleep(2500);
			players.nameOfPlayers(score);
		}
	}
};

class Menu {
private:
	Player Name;
	Game CrossRoad;
	sf::Music background_music;
	sf::Music win;
	sf::Music defeat;
	int playerY = 6;
	int playerX = 20;
	
public:
	void drawTutorialField(bool running, bool victory, bool collided) {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		string map[] = {
			"|~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=|",
			"|......................................|",
			"|~~~~~====~~~~~~=====~~~~~=====~~~~====|",
			"|......................................|",
			"|..#####......................#####....|",
			"|...............####...................|",
			"|......................................|"
		};
		
		for (int i = 0; i < 7; i++) {
			SetConsoleCursorPosition(hConsole, {1, (SHORT)(i + 7)});
			for (int j = 0; j < map[i].length(); j++) {
				if (i == playerY && j == playerX) {
					if (collided) {
						setColor(12);
						cout << "X";
					}
					else {
						setColor(10);
						cout << "P";
					}
				}
				else {
					char symbol = map[i][j];
					switch (symbol) {
						case '#': setColor(15); break;
						case '~': setColor(1); break;
						case '=': setColor(4);  break;
						case '.': setColor(8);  break;
						default:  setColor(7);  break;
					}
					cout << symbol;
				}
			}
		}
		setColor(7);
		cout << "\n <======================================>";
		cout << "\n |  (1). Exit                           |";
		cout << "\n <======================================>";
	}

	void Tutorial() {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		if (!defeat.openFromFile("music/defeat.mp3")) {
			cout << "Failed to load defeat music!" << endl;
		}
		if (!win.openFromFile("music/victory.mp3")) {
			cout << "Failed to load defeat music!" << endl;
		}
		
		string header[] = {
			"<======================================>",
			"| .=. r-. .=. .=. .=.  r-. .=. .=. r=. |",
			"| l   t<' l l '=. '=.  t<' l l t-t l l |",
			"| '=^ l l '=' '=' '='  l l '=' l l t=' |",
			"<======================================>"
		};

		system("cls");
		
		bool frame = false;
		bool victory = false;
		bool collided = false;

		for (int i = 0; i < 5; i++) {
			SetConsoleCursorPosition(hConsole, {1, (SHORT)(i + 2)});
			cout << header[i];
		}
		drawTutorialField(true, victory, collided);

		while (true) {
			string map[] = {
				"|~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=|",
				"|......................................|",
				"|~~~~~====~~~~~~=====~~~~~=====~~~~====|",
				"|......................................|",
				"|..#####......................#####....|",
				"|...............####...................|",
				"|......................................|"
			};
			collided = false;
			Sleep(25);

			drawTutorialField(true, victory, collided);
			Name.playerMovement(playerX, playerY);
			if (playerX < 1) playerX = 1;
			if (playerX > 38) playerX = 38;

			if (playerY < 0) playerY = 0;
			if (playerY > 6) playerY = 6;

			Sleep(10);

			char standingOn = map[playerY][playerX];
			bool isRiver = (playerY == 2);
			bool isTruck = (playerY == 4 || playerY == 5);

			if ((isRiver && standingOn == '~') ||(isTruck && standingOn == '#')) {
				collided = true;
				drawTutorialField(true, victory, collided);
				SetConsoleCursorPosition(hConsole, {13, 10});
				setColor(12);
				cout << "l^ lE  A  l^ ||";
				SetConsoleCursorPosition(hConsole, {13, 11});
				cout << "lo lL /H\\ lo !!";

				setColor(7);
				defeat.play();
				Sleep(1300);
				playerY = 6;
				playerX = 20;
			}

			if (playerY == 0 && (standingOn == '~' || standingOn == '=')) {
				drawTutorialField(true, victory, collided);

				SetConsoleCursorPosition(hConsole, {13, 10});
				setColor(10);
				cout << "H.H TT lL| ||";
				SetConsoleCursorPosition(hConsole, {13, 11});
				cout << "V'V ll l'i !!";
				
				setColor(7);
				win.play();
				Sleep(2000);
				playerY = 6;
				playerX = 20;
			}

			if (_kbhit()) {
				char choice = _getch();
				if (choice == '1') {
					return;
				}
			}
		}
	}
	void Credits() {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		int startX = 1;
		int startY = 2; 

		string creditsOne[] = {
			"<======================================>",
			"| .=. r-. .=. .=. .=.  r-. .=. .=. r=. |",
			"| l   t<' l l '=. '=.  t<' l l t-t l l |",
			"| '=^ l l '=' '=' '='  l l '=' l l t=' |",
			"<======================================>",
			"|  Credits:                   _,=.     |",
			"|    Albeus, Rex           .=r..__h=.  |",
			"|    Balondo, Renz        |-[//]==[//] |",
			"|    Cepeda, Nico         |      .>  | |",
			"|    Sto.Domingo, Lander  |==..__..==| |",
			"|  (1). Exit                           |",
			"<======================================>"
		};

		string creditsTwo[] = {
			"<======================================>",
			"| .=. r-. .=. .=. .=.  r-. .=. .=. r=. |",
			"| l   t<' l l '=. '=.  t<' l l t-t l l |",
			"| '=^ l l '=' '=' '='  l l '=' l l t=' |",
			"<======================================>",
			"|  Credits:                     _,_    |",
			"|    Albeus, Rex           .===l   '=. |",
			"|    Balondo, Renz        |     '-=' | |",
			"|    Cepeda, Nico         |-[//]==[//] |",
			"|    Sto.Domingo, Lander  |..__  '> _| |",
			"|  (1). Exit                           |",
			"<======================================>"
		};

		string starsOne[] = {
			" ^ ",
			"<#>",
			" v "
		};

		string starsTwo[] = {
			"  A",
			"<=#=>",
			"  V"
		};

		system("cls");

		bool frame = false;

		while (true) {
			for (int i = 0; i < 12; i++) {
				SetConsoleCursorPosition(hConsole, { (SHORT)startX, (SHORT)(startY + i)});
				if (!frame)	{
					cout << creditsOne[i];
				}
				else {
					cout << creditsTwo[i];
				}
			}
			for (int i = 0; i < 3; i++) {
				setColor(6);
				if (!frame)	{
					SetConsoleCursorPosition(hConsole, { (SHORT)27, (SHORT)7});
					cout << "^";
					SetConsoleCursorPosition(hConsole, { (SHORT)26, (SHORT)8});
					cout << "<#>";
					SetConsoleCursorPosition(hConsole, { (SHORT)27, (SHORT)9});
					cout << "v";
				}
				else {
					SetConsoleCursorPosition(hConsole, { (SHORT)27, (SHORT)7});
					cout << "A";
					SetConsoleCursorPosition(hConsole, { (SHORT)25, (SHORT)8});
					cout << "<=#=>";
					SetConsoleCursorPosition(hConsole, { (SHORT)27, (SHORT)9});
					cout << "V";
				}
				setColor(7);
			}
			
			frame = !frame;

			Sleep(435);

			if (_kbhit()) {
				char choice = _getch();
				if (choice == '1') {
					return;
				}
			}
		}
	}
	void Leaderboards() {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		system("cls");

		string header[] = {
			"<======================================>",
			"| .=. r-. .=. .=. .=.  r-. .=. .=. r=. |",
			"| l   t<' l l '=. '=.  t<' l l t-t l l |",
			"| '=^ l l '=' '=' '='  l l '=' l l t=' |",
			"<======================================>",
			"|  TOP 5 LEADERBOARDS                  |"
		};

		string victoryOne[] = {
			"  .     |\\/\\/|  ' ",
			"    . `_|____| `. ",
			"   |l ' ^_  ^ ' |l",
			"   'lh   V     |/ ",
			"     H=.__..===H  "
		};

		string victoryTwo[] = {
			"  ,     |\\/\\/| `  ",
			"     ` _|____l  '",
			"   .  ' ^_  ^ ' . ",
			"   l\\|   V     |l|",
			"    \\H=.__..===H/ "
		};

		bool frame = false;
		
		while (true) {
			for (int i = 0; i < 6; i++) {
				SetConsoleCursorPosition(hConsole, {1, (SHORT)(2 + i)});
				cout << header[i];
			}

			ifstream inFile("leaderboard.txt");
			string name;
			int score;
			int rank = 1;
			int row = 8;

			if (!inFile || inFile.peek() == EOF) {
				SetConsoleCursorPosition(hConsole, {1, (SHORT)row});
				cout << "|    No scores yet!";
				SetConsoleCursorPosition(hConsole, {40, (SHORT)row});
				cout << "|";
			}
			else {
				while (inFile >> name >> score && rank <= 5) {
					SetConsoleCursorPosition(hConsole, {1, (SHORT)row});
					cout << "|    " << rank << ". " << name << " - " << score << " pts";

					SetConsoleCursorPosition(hConsole, {40, (SHORT)row});
					cout << "|";
					rank++;
					row++;
				}
				inFile.close();
			}

			while (rank <= 5) {
				SetConsoleCursorPosition(hConsole, {1, (SHORT)row});
				cout << "|";
				SetConsoleCursorPosition(hConsole, {40, (SHORT)row});
				cout << "|";
				rank++;
				row++;
			}

			SetConsoleCursorPosition(hConsole, {1, (SHORT)(row)});
			cout << "|  (1). Exit";
			SetConsoleCursorPosition(hConsole, {40, (SHORT)(row)});
			cout << "|";
			SetConsoleCursorPosition(hConsole, {1, (SHORT)(row + 1)});
			cout << "<=====================================>";
			
			for (int i = 0; i < 5; i++) {
				SetConsoleCursorPosition(hConsole, { (SHORT)21, (SHORT)(8 + i) });
				setColor(14);
				if (!frame)	{
					cout << victoryOne[i];
				}
				else {
					cout << victoryTwo[i];
				}
				setColor(7);
			}
			frame = !frame;

			Sleep(435);

			if (_kbhit()) {
				char choice = _getch();
				if (choice == '1') {
					return;
				}
			}
		}
	}
	bool TitleScreen() {
		system("cls");
		if (!background_music.openFromFile("music/wonder_run.mp3")) {
			cout << "Error: Could not find the music file!" << endl;
		}
		else {
			background_music.setLooping(true);
			background_music.setVolume(100.f);
			background_music.play();
		}
		
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		int startX = 1;
		int startY = 2; 

		Game CrossRoad;

		string chickenOne[] = {
			"<======================================>",
			"| .=. r-. .=. .=. .=.  r-. .=. .=. r=. |",
			"| l   t<' l l '=. '=.  t<' l l t-t l l |",
			"| '=^ l l '=' '=' '='  l l '=' l l t=' |",
			"<======================================>",
			"|  1. Play                 c^-~.       |",
			"|  2. Tutorial            .\"====h.     |",
			"|  3. Leaderboards       | o      l    |",
			"|  4. Credits           <|        1    |",
			"|  5. Quit              'l--.._   1    |",
			"<======================================>",
			"|  Dangerous Objects:     Goal/Finish: |",
			"|   '#' truck |'~' river   '~=~=~=~='  |",
			"|                                      |",
			"|  Safe Objects:                       |",
			"|   '.' road  |'=' log                 |",
			"|                                      |",
			"|  Movement:                           |",
			"|   ^ = Up   > = Right                 |",
			"|   v = Down < = Left                  |",
			"|                                      |",
			"<======================================>"
		};

		string chickenTwo[] = {
			"<======================================>",
			"| .=. r-. .=. .=. .=.  r-. .=. .=. r=. |",
			"| l   t<' l l '=. '=.  t<' l l t-t l l |",
			"| '=^ l l '=' '=' '='  l l '=' l l t=' |",
			"<======================================>",
			"|  1. Play          ,1    .\"====h.     |",
			"|  2. Tutorial      ||  _l O      l    |",
			"|  3. Leaderboards  l'  >|        l    |",
			"|  4. Credits       '   'l--.._   1    |",
			"|  5. Quit          o   /       ''''1  |",
			"<======================================>",
			"|  Dangerous Objects:     Goal/Finish: |",
			"|   '#' truck |'~' river   '~=~=~=~='  |",
			"|                                      |",
			"|  Safe Objects:                       |",
			"|   '.' road  |'=' log                 |",
			"|                                      |",
			"|  Movement:                           |",
			"|   ^ = Up   > = Right                 |",
			"|   v = Down < = Left                  |",
			"|                                      |",
			"<======================================>"
		};

		bool frame = false;

		while (true) {
			for (int i = 0; i < 22; i++) {
				SetConsoleCursorPosition(hConsole, { (SHORT)startX, (SHORT)(startY + i)});
				if (!frame)	cout << chickenOne[i];
				else cout << chickenTwo[i];
			}
			frame = !frame;

			Sleep(435);

			if (_kbhit()) {
				char choice = _getch();
				switch (choice){
					case '1': {
						background_music.stop();

						system("cls");
						CrossRoad.Layout();
						CrossRoad.GameRunning();

						background_music.play();
						background_music.setVolume(100.f);

						system("cls");
						break;
					}
					case '2': {
						Tutorial();
						break;
					}
					case '3': {
						Leaderboards();
						break;
					}
					case '4': {
						Credits();
						break;
					}
					case '5': {
						background_music.stop();
						return false;
						break;
					}
				}
			}
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