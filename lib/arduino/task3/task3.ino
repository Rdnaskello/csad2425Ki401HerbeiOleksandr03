/**
 * @file main.ino
 * @brief Arduino backend for Tic-Tac-Toe game with AI and LED indicators.
 */
#include <Arduino.h>
#include <EEPROM.h> // Для збереження стану діодів між перезавантаженнями

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
char receivedData[10];

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
    Serial.begin(4800);
    pinMode(BlueledPin, OUTPUT);
    pinMode(YellowledPin, OUTPUT);
    loadLedStateFromEEPROM(); // Завантаження стану діодів
    while (Serial.available() > 0) {
        Serial.read(); // Чистимо серійний буфер
    }
    resetBoard(); 
}


/**
 * @brief Arduino main loop.
 * Handles serial input and game logic.
 */
void loop() {
    // Читання серійної команди
    if (Serial.available() > 0) {
        char receivedChar = Serial.read();
        if (receivedChar == '\n') {
            receivedData[dataIndex] = '\0'; // Завершуємо рядок
            processCommand(); // Обробка команди
            dataIndex = 0;    // Скидання індексу після обробки
        } else if (dataIndex < sizeof(receivedData) - 1) {
            receivedData[dataIndex++] = receivedChar;
        } else {
            dataIndex = 0; // Скидання буфера при переповненні
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
        memset(receivedData, 0, sizeof(receivedData)); // Очищення буфера
        return;
    }

    if (strcmp(receivedData, "My move:") == 0) {
        Serial.println("Nice but");
    }

    if (strcmp(receivedData, "BLed") == 0) {
        BlueblinkLED();
        blueLedState = !blueLedState; // Змінюємо стан синього діода
        saveLedStateToEEPROM(); // Зберігаємо стан у EEPROM
        
    } 
    if (strcmp(receivedData, "Yled") == 0) {
        YellowblinkLED();
        yellowLedState = !yellowLedState; // Змінюємо стан жовтого діода
        saveLedStateToEEPROM(); // Зберігаємо стан у EEPROM
        
    } 
    // Якщо отримана команда - reset
    if (strcmp(receivedData, "reset") == 0) {
        DrawblinkLED();
        resetBoard();
        waitingForPlayerMove = false;
        pvpmode = false; // Вихід із PvP
        playerTurn = true; // Починаємо з першого гравця
        return;
    }

    // Якщо гра ще не завершена
    if (!gameOver) {
        // Обробка режимів гри
        if (strcmp(receivedData, "player") == 0) {
            sendCurrentBoardState();
            waitingForPlayerMove = true; // Очікуємо хід гравця
            pvpmode = false; // Вимикаємо PvP
            memset(receivedData, 0, sizeof(receivedData));
            return;
        } else if (strcmp(receivedData, "ai") == 0) {
            makeAIMove();
            sendCurrentBoardState();
            waitingForPlayerMove = true; // Після ходу AI чекаємо на хід гравця
            pvpmode = false;
            memset(receivedData, 0, sizeof(receivedData));
            return;
        } else if (strcmp(receivedData, "pvp") == 0) {
            pvpmode = true;
            waitingForPlayerMove = false; // Вимикаємо інші режими
            playerTurn = true; // Перший гравець X
            sendCurrentBoardState();
            memset(receivedData, 0, sizeof(receivedData));
            return;
        }

        // Обробка ходу (один метод для всіх режимів)
        if (receivedData[1] == ',' && receivedData[0] >= '0' &&
            receivedData[0] <= '2' && receivedData[2] >= '0' &&
            receivedData[2] <= '2') {
            
            int row = receivedData[0] - '0';
            int col = receivedData[2] - '0';

            if (board[row][col] == ' ') { // Якщо клітинка порожня
                // У PvP змінюємо черговість гравців
                if (pvpmode) {
                    board[row][col] = playerTurn ? 'X' : 'O';
                    playerTurn = !playerTurn; // Змінюємо гравця
                } else {
                    board[row][col] = 'X'; // Хід гравця X
                    makeAIMove(); // Хід AI
                    
                }
                moveCount++;
                sendCurrentBoardState();

                // Перевірка результату гри
                if (checkWinner()) {
                    gameOver = true;
                    if (pvpmode) {
                        // PvP режим
                        if (playerTurn) {
                            YellowblinkLED(); // Жовтий діод для гравця O
                            Serial.println("O win!");
                        } else {
                            BlueblinkLED(); // Синій діод для гравця X
                            Serial.println("X win!");
                        }
                    } else {
                        // AI режим
                        if (isAIMoveWinning()) { 
                            YellowblinkLED(); // Жовтий діод для AI
                            Serial.println("AI win!");
                        } else {
                            BlueblinkLED(); // Синій діод для гравця
                            Serial.println("You win!");
                        }
                    }
                } else if (moveCount >= 9) {
                    gameOver = true;
                    DrawblinkLED(); // Обидва діоди блимають
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

    // Перевірка, чи значення є допустимим
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
    return evaluate(board) == 1; // 1 означає виграш AI
}

/**
 * @brief Executes the AI's move.
 * Finds the best move for the AI using the minimax algorithm.
 * @return The best move as a Pair (row, column).
 */
Pair makeAIMove() {
    Pair bestMove = findBestMove(board); // Знаходимо найкращий хід
    board[bestMove.first][bestMove.second] = 'O'; // AI робить хід за O
    moveCount++; // Збільшуємо лічильник ходів
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
    isPlayerOneTurn = true; // Додано
    memset(receivedData, 0, sizeof(receivedData));
    sendCurrentBoardState();
}

/**
 * @brief Sends the current state of the game board via serial.
 */
void sendCurrentBoardState() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            Serial.print(board[i][j]); // Відправка символу
        }
    }
    Serial.println(); // Перехід на новий рядок
}

/**
 * @brief Checks if there is a winner on the game board.
 * @return True if there is a winner, false otherwise.
 */
bool checkWinner() {
    // Перевірка горизонтальних та вертикальних ліній
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != ' ') return true;
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != ' ') return true;
    }
    // Перевірка діагоналей
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != ' ') return true;
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != ' ') return true;
    return false;
}

/**
 * @brief Blinks the blue LED.
 */
void BlueblinkLED() {
    if (blueLedState) { // Виконуємо тільки якщо діод увімкнений у налаштуваннях
        digitalWrite(BlueledPin, HIGH);
        delay(300);
        digitalWrite(BlueledPin, LOW);
    }
}

/**
 * @brief Blinks the yellow LED.
 */
void YellowblinkLED() {
    if (yellowLedState) { // Виконуємо тільки якщо діод увімкнений у налаштуваннях
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
    // Перевірка рядків і стовпців
    for (int i = 0; i < 3; i++) {
        if (board[i][0] != ' ' && board[i][0] == board[i][1] && board[i][1] == board[i][2])
            return (board[i][0] == ai) ? 1 : -1;
        if (board[0][i] != ' ' && board[0][i] == board[1][i] && board[1][i] == board[2][i])
            return (board[0][i] == ai) ? 1 : -1;
    }
    // Перевірка діагоналей
    if (board[0][0] != ' ' && board[0][0] == board[1][1] && board[1][1] == board[2][2])
        return (board[0][0] == ai) ? 1 : -1;
    if (board[0][2] != ' ' && board[0][2] == board[1][1] && board[1][1] == board[2][0])
        return (board[0][2] == ai) ? 1 : -1;

    return 0; // Нічия
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
    if (depth > 9) return 0; // Ліміт на глибину
    int score = evaluate(board);

    if (score == 1 || score == -1) return score; // Якщо є перемога
    if (!isMovesLeft(board)) return 0; // Якщо нічия

    if (isMaximizing) {
        int best = -1000;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == ' ') {
                    board[i][j] = ai; // Пробуємо хід AI
                    best = max(best, minimax(board, depth + 1, false, alpha, beta));
                    board[i][j] = ' '; // Відміняємо хід
                    alpha = max(alpha, best);
                    if (beta <= alpha) break; // Обрізання
                }
            }
        }
        return best;
    } else {
        int best = 1000;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == ' ') {
                    board[i][j] = player; // Пробуємо хід гравця
                    best = min(best, minimax(board, depth + 1, true, alpha, beta));
                    board[i][j] = ' '; // Відміняємо хід
                    beta = min(beta, best);
                    if (beta <= alpha) break; // Обрізання
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
        {3, 2, 3}, // Пріоритетність позицій
        {2, 4, 2},
        {3, 2, 3}
    };
    Pair winningMove = findWinningMove(board, ai);
    if (winningMove.first != -1) {
        return winningMove; // Виконуємо виграшний хід
    }
    // Спершу перевіряємо, чи є хід, який блокує перемогу гравця
    Pair blockingMove = findBlockingMove(board, player);
    if (blockingMove.first != -1) {
        return blockingMove; // Повертаємо блокувальний хід
    }
    Pair forkMove = findForkMove(board, ai);
    if (forkMove.first != -1) {
        return forkMove; // Повертаємо хід, який створює форк
    }
    

    // MiniMax для визначення найкращого ходу
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == ' ') { // Якщо клітинка порожня
                board[i][j] = ai; // Пробуємо хід AI
                int moveVal = minimax(board, 0, false, -1000, 1000);
                board[i][j] = ' '; // Відміняємо хід

                // Додаємо оцінку пріоритету позиції
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
            if (board[i][j] == ' ') { // Якщо клітинка порожня
                board[i][j] = playerSymbol; // Симулюємо хід
                if (evaluate(board) == 1) { // Перевіряємо, чи це виграш
                    board[i][j] = ' '; // Скидаємо хід
                    return {i, j}; // Повертаємо виграшний хід
                }
                board[i][j] = ' '; // Скидаємо хід
            }
        }
    }
    return {-1, -1}; // Немає виграшних ходів
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
            if (board[i][j] == ' ') { // Якщо клітинка порожня
                board[i][j] = opponent; // Симулюємо хід опонента
                if (evaluate(board) == -1) { // Якщо це виграшний хід
                    board[i][j] = ' '; // Відміняємо хід
                    return {i, j}; // Повертаємо блокувальний хід
                }
                board[i][j] = ' '; // Відміняємо хід
            }
        }
    }
    return {-1, -1}; // Немає загроз
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
            if (board[i][j] == ' ') { // Якщо клітинка порожня
                board[i][j] = playerSymbol; // Симулюємо хід
                if (canCreateFork(board, playerSymbol)) {
                    board[i][j] = ' '; // Скидаємо хід
                    return {i, j}; // Повертаємо хід, який створює форк
                }
                board[i][j] = ' '; // Скидаємо хід
            }
        }
    }
    return {-1, -1}; // Форк неможливий
}
