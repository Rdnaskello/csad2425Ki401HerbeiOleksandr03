/**
 * @file main.ino
 * @brief Arduino backend for Tic-Tac-Toe game with AI and LED indicators.
 */
#include <Arduino.h>
#include <EEPROM.h> 
#include <AUnit.h>

/**
 * @struct Pair
 * @brief Represents a pair of coordinates (row and column) for the game board.
 */
struct Pair {
    int first;///< Row index
    int second;///< Column index
};

/// Pin for the blue LED.
const int BlueledPin = 8; 

/// Pin for the yellow LED.
const int YellowledPin = 9;

/// Buffer for incoming serial data.
char receivedData[20];

/// Index for the serial data buffer.
int dataIndex = 0;

/// Game board (3x3 grid).
char board[3][3] = {{' ', ' ', ' '}, {' ', ' ', ' '}, {' ', ' ', ' '}}; // gameBoard
bool gameOver = false; ///< Flag to indicate if the game is over.
int moveCount = 0; ///< Counter for the number of moves made.
int blinkCount = 0; ///< Counter for LED blinks.
bool ledState = LOW;///< State of the LED.
char ai = 'O'; ///< Character representing the AI player.
char player = 'X'; ///< Character representing the human player.
bool waitingForPlayerMove = false; ///< Flag to indicate if waiting for player's move.
bool isPlayerOneTurn = true; ///< Indicates if it is player one's turn.
bool pvpmode = false; ///< Flag for Player vs Player mode.
bool playerTurn = true; ///< Current player's turn (true = Player X, false = Player O).
bool blueLedState = false; ///< State of the blue LED.
bool yellowLedState = false; ///< State of the yellow LED.


/**
 * @brief Arduino setup function.
 * Initializes the serial communication, pins, and loads LED states.
 */
void setup(){
    Serial.begin(9600);
    pinMode(BlueledPin, OUTPUT);
    pinMode(YellowledPin, OUTPUT);
    loadLedStateFromEEPROM(); 
    while (Serial.available() > 0) {
        Serial.read(); 
    }
    resetBoard(); 
}


/**
 * @brief Arduino main loop.
 * Handles serial input and game logic.
 */
void loop() {
    aunit::TestRunner::setVerbosity(aunit::Verbosity::kAll);
    Serial.println("Starting AUnit tests...");
    aunit::TestRunner::run();
    
    if (Serial.available() > 0) {
        char receivedChar = Serial.read();
        if (receivedChar == '\n') {
            receivedData[dataIndex] = '\0';
            processCommand(); 
            dataIndex = 0;    
        } else if (dataIndex < sizeof(receivedData) - 1) {
            receivedData[dataIndex++] = receivedChar;
        } else {
            dataIndex = 0; 
            memset(receivedData, 0, sizeof(receivedData));
        }
    }
}


/**
 * @brief Processes the received command.
 * Interprets and executes commands sent via serial communication.
 */
void processCommand() {
    if (strlen(receivedData) > 9) {
        Serial.println("Error: Command too long!");
        memset(receivedData, 0, sizeof(receivedData)); 
        return;
    }

    if (strcmp(receivedData, "BLed") == 0) {
        BlueblinkLED();
        blueLedState = !blueLedState; 
        saveLedStateToEEPROM(); 
        
    } 
    if (strcmp(receivedData, "Yled") == 0) {
        YellowblinkLED();
        yellowLedState = !yellowLedState; 
        saveLedStateToEEPROM(); 
        
    } 
    if (strcmp(receivedData, "reset") == 0) {
        DrawblinkLED();
        resetBoard();
        waitingForPlayerMove = false;
        pvpmode = false; 
        playerTurn = true; 
        return;
    }

    if (!gameOver) {
        if (strcmp(receivedData, "player") == 0) {
            sendCurrentBoardState();
            waitingForPlayerMove = true; 
            pvpmode = false;
            memset(receivedData, 0, sizeof(receivedData));
            return;
        } else if (strcmp(receivedData, "ai") == 0) {
            makeAIMove();
            sendCurrentBoardState();
            waitingForPlayerMove = true;
            pvpmode = false;
            memset(receivedData, 0, sizeof(receivedData));
            return;
        } else if (strcmp(receivedData, "pvp") == 0) {
            pvpmode = true;
            waitingForPlayerMove = false; 
            playerTurn = true;
            sendCurrentBoardState();
            memset(receivedData, 0, sizeof(receivedData));
            return;
        }
        if (receivedData[1] == ',' && receivedData[0] >= '0' &&
            receivedData[0] <= '2' && receivedData[2] >= '0' &&
            receivedData[2] <= '2') {
            
            int row = receivedData[0] - '0';
            int col = receivedData[2] - '0';

            if (board[row][col] == ' ') { 
                if (pvpmode) {
                    board[row][col] = playerTurn ? 'X' : 'O';
                    playerTurn = !playerTurn; 
                } else {
                    board[row][col] = 'X'; 
                    makeAIMove();
                    
                }
                moveCount++;
                sendCurrentBoardState();
                if (checkWinner()) {
                    gameOver = true;
                    if (pvpmode) {
                        if (playerTurn) {
                            YellowblinkLED(); 
                            Serial.println("O win!");
                        } else {
                            BlueblinkLED(); 
                            Serial.println("X win!");
                        }
                    } else {
                        if (isAIMoveWinning()) { 
                            YellowblinkLED();
                            Serial.println("AI win!");
                        } else {
                            BlueblinkLED(); 
                            Serial.println("You win!");
                        }
                    }
                } else if (moveCount >= 9) {
                    gameOver = true;
                    DrawblinkLED(); 
                    Serial.println("Draw!");
                }
            }
        }
        memset(receivedData, 0, sizeof(receivedData));
        dataIndex = 0;
    }
}

/**
 * @brief Saves the LED states to EEPROM.
 */
void saveLedStateToEEPROM() {
    if (EEPROM.read(0) != blueLedState) {
        EEPROM.write(0, blueLedState);
        
    }
    if (EEPROM.read(1) != yellowLedState) {
        EEPROM.write(1, yellowLedState);
        
    }
}

/**
 * @brief Loads the LED states from EEPROM.
 */
void loadLedStateFromEEPROM() {
    int blueState = EEPROM.read(0);
    int yellowState = EEPROM.read(1);

    blueLedState = (blueState == 1);
    yellowLedState = (yellowState == 1);

    digitalWrite(BlueledPin, blueLedState ? HIGH : LOW);
    digitalWrite(YellowledPin, yellowLedState ? HIGH : LOW);
}

/**
 * @brief Checks if the AI's move results in a win.
 * @return True if the AI's move is a winning move, false otherwise.
 */
bool isAIMoveWinning() {
    return evaluate(board) == 1; 
}

/**
 * @brief Executes the AI's move.
 * Finds the best move for the AI using the minimax algorithm.
 * @return The best move as a Pair (row, column).
 */
Pair makeAIMove() {
    Pair bestMove = findBestMove(board); 
    board[bestMove.first][bestMove.second] = 'O';
    moveCount++; 
    return bestMove;
}

/**
 * @brief Resets the game board to its initial state.
 */
void resetBoard() {   
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            board[i][j] = ' ';
        }
    }
    moveCount = 0;
    gameOver = false;
    waitingForPlayerMove = false;
    pvpmode = false;
    playerTurn = true;
    isPlayerOneTurn = true; 
    memset(receivedData, 0, sizeof(receivedData));
    sendCurrentBoardState();
}

/**
 * @brief Sends the current state of the game board via serial.
 */
void sendCurrentBoardState() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            Serial.print(board[i][j]); 
        }
    }
    Serial.println(); 
}

/**
 * @brief Checks if there is a winner on the game board.
 * @return True if there is a winner, false otherwise.
 */
bool checkWinner() {
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != ' ') return true;
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != ' ') return true;
    }
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != ' ') return true;
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != ' ') return true;
    return false;
}

/**
 * @brief Blinks the blue LED.
 */
void BlueblinkLED() {
    if (blueLedState) { 
        digitalWrite(BlueledPin, HIGH);
        delay(300);
        digitalWrite(BlueledPin, LOW);
    }
}

/**
 * @brief Blinks the yellow LED.
 */
void YellowblinkLED() {
    if (yellowLedState) { 
        digitalWrite(YellowledPin, HIGH);
        delay(300);
        digitalWrite(YellowledPin, LOW);
    }
}

/**
 * @brief Blinks both LEDs in case of a draw.
 */
void DrawblinkLED() {
    if (!blueLedState && !yellowLedState) return;
    if (blueLedState && yellowLedState){
      digitalWrite(BlueledPin, HIGH);
      digitalWrite(YellowledPin, HIGH);
      delay(300);
      digitalWrite(BlueledPin, LOW);
      digitalWrite(YellowledPin, LOW);
    } else if (blueLedState && !yellowLedState) { 
        digitalWrite(BlueledPin, HIGH);
        delay(300);
        digitalWrite(BlueledPin, LOW);
    } else if (yellowLedState && !blueLedState){
        digitalWrite(YellowledPin, HIGH);
        delay(300);
        digitalWrite(YellowledPin, LOW);
    }
}


/**
 * @brief Evaluates the current board state.
 * @param board The game board.
 * @return 1 if AI wins, -1 if player wins, 0 otherwise.
 */
int evaluate(char board[3][3]) {
    for (int i = 0; i < 3; i++) {
        if (board[i][0] != ' ' && board[i][0] == board[i][1] && board[i][1] == board[i][2])
            return (board[i][0] == ai) ? 1 : -1;
        if (board[0][i] != ' ' && board[0][i] == board[1][i] && board[1][i] == board[2][i])
            return (board[0][i] == ai) ? 1 : -1;
    }
    if (board[0][0] != ' ' && board[0][0] == board[1][1] && board[1][1] == board[2][2])
        return (board[0][0] == ai) ? 1 : -1;
    if (board[0][2] != ' ' && board[0][2] == board[1][1] && board[1][1] == board[2][0])
        return (board[0][2] == ai) ? 1 : -1;

    return 0; 
}

/**
 * @brief Checks if there are any empty cells left on the board.
 * @param board The game board.
 * @return True if there are empty cells, false otherwise.
 */
bool isMovesLeft(char board[3][3]) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] == ' ') return false;
    return true;
}

/**
 * @brief Checks if a player can create a fork (two winning moves).
 * @param board The game board.
 * @param playerSymbol The symbol of the player ('X' or 'O').
 * @return True if a fork is possible, false otherwise.
 */
bool canCreateFork(char board[3][3], char playerSymbol) {
    int winningMoves = 0;

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (board[i][j] == ' ') {
                board[i][j] = playerSymbol;
                if (evaluate(board) == ((playerSymbol == ai) ? 1 : -1)) {
                    winningMoves++;
                }
                board[i][j] = ' ';
            }
        }
    }
    return winningMoves >= 2;
}

/**
 * @brief Implements the minimax algorithm with alpha-beta pruning.
 * @param board The game board.
 * @param depth Current depth in the game tree.
 * @param isMaximizing True if the AI is maximizing its score, false otherwise.
 * @param alpha Alpha value for pruning.
 * @param beta Beta value for pruning.
 * @return The evaluated score of the board.
 */
int minimax(char board[3][3], int depth, bool isMaximizing, int alpha, int beta) {
    if (depth > 9) return 0; 
    int score = evaluate(board);

    if (score == 1 || score == -1) return score; 
    if (!isMovesLeft(board)) return 0; 

    if (isMaximizing) {
        int best = -1000;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == ' ') {
                    board[i][j] = ai; 
                    best = max(best, minimax(board, depth + 1, false, alpha, beta));
                    board[i][j] = ' '; 
                    alpha = max(alpha, best);
                    if (beta <= alpha) break; 
                }
            }
        }
        return best;
    } else {
        int best = 1000;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == ' ') {
                    board[i][j] = player; 
                    best = min(best, minimax(board, depth + 1, true, alpha, beta));
                    board[i][j] = ' '; 
                    beta = min(beta, best);
                    if (beta <= alpha) break; 
                }
            }
        }
        return best;
    }
}

/**
 * @brief Finds the best move for the AI using evaluation and minimax.
 * @param board The game board.
 * @return The best move as a Pair (row, column).
 */
Pair findBestMove(char board[3][3]) {
    Pair bestMove = {-1, -1};
    int bestVal = -1000;
    int positionPriority[3][3] = {
        {3, 2, 3},
        {2, 4, 2},
        {3, 2, 3}
    };
    Pair winningMove = findWinningMove(board, ai);
    if (winningMove.first != -1) {
        return winningMove; 
    }
    Pair blockingMove = findBlockingMove(board, player);
    if (blockingMove.first != -1) {
        return blockingMove;
    }
    Pair forkMove = findForkMove(board, ai);
    if (forkMove.first != -1) {
        return forkMove;
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == ' ') { 
                board[i][j] = ai; 
                int moveVal = minimax(board, 0, false, -1000, 1000);
                board[i][j] = ' '; 
                moveVal += positionPriority[i][j];

                if (moveVal > bestVal) {
                    bestMove = {i, j};
                    bestVal = moveVal;
                }
            }
        }
    }
    return bestMove;
}

/**
 * @brief Finds a winning move for a given player.
 * @param board The game board.
 * @param playerSymbol The player's symbol ('X' or 'O').
 * @return The winning move as a Pair (row, column), or {-1, -1} if none found.
 */
Pair findWinningMove(char board[3][3], char playerSymbol) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (board[i][j] == ' ') { 
                board[i][j] = playerSymbol; 
                if (evaluate(board) == 1) { 
                    board[i][j] = ' '; 
                    return {i, j};
                }
                board[i][j] = ' '; 
            }
        }
    }
    return {-1, -1}; 
}

/**
 * @brief Finds a move to block the opponent's win.
 * @param board The game board.
 * @param opponent The opponent's symbol ('X' or 'O').
 * @return The blocking move as a Pair (row, column), or {-1, -1} if none found.
 */
Pair findBlockingMove(char board[3][3], char opponent) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (board[i][j] == ' ') {
                board[i][j] = opponent; 
                if (evaluate(board) == -1) { 
                    board[i][j] = ' '; 
                    return {i, j}; 
                }
                board[i][j] = ' ';
            }
        }
    }
    return {-1, -1};
}

/**
 * @brief Finds a move that creates a fork for a player.
 * @param board The game board.
 * @param playerSymbol The player's symbol ('X' or 'O').
 * @return The fork move as a Pair (row, column), or {-1, -1} if none found.
 */
Pair findForkMove(char board[3][3], char playerSymbol) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (board[i][j] == ' ') { 
                board[i][j] = playerSymbol; 
                if (canCreateFork(board, playerSymbol)) {
                    board[i][j] = ' '; 
                    return {i, j}; 
                }
                board[i][j] = ' '; 
            }
        }
    }
    return {-1, -1};
}
