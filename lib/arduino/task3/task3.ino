#include <Arduino.h>
#include <EEPROM.h> // Для збереження стану діодів між перезавантаженнями

struct Pair {
    int first;
    int second;
};

const int BlueledPin = 8; // Pin for Led
const int YellowledPin = 9; 
char receivedData[10];
int dataIndex = 0;
char board[3][3] = {{' ', ' ', ' '}, {' ', ' ', ' '}, {' ', ' ', ' '}}; // gameBoard
bool gameOver = false;
int moveCount = 0;
int blinkCount = 0; 
bool ledState = LOW;
char ai = 'O';
char player = 'X';
bool waitingForPlayerMove = false;
bool isPlayerOneTurn = true; // Змінна для відстеження черги ходу гравців
bool pvpmode = false;
bool playerTurn = true; // Хто ходить: true - гравець X, false - гравець O
bool blueLedState = false; 
bool yellowLedState = false;

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

// Функція для збереження стану діодів у EEPROM
void saveLedStateToEEPROM() {
    if (EEPROM.read(0) != blueLedState) {
        EEPROM.write(0, blueLedState);
        
    }
    if (EEPROM.read(1) != yellowLedState) {
        EEPROM.write(1, yellowLedState);
        
    }
}

// Функція для завантаження стану діодів із EEPROM
void loadLedStateFromEEPROM() {
    int blueState = EEPROM.read(0);
    int yellowState = EEPROM.read(1);

    // Перевірка, чи значення є допустимим
    blueLedState = (blueState == 1);
    yellowLedState = (yellowState == 1);

    digitalWrite(BlueledPin, blueLedState ? HIGH : LOW);
    digitalWrite(YellowledPin, yellowLedState ? HIGH : LOW);
}

bool isAIMoveWinning() {
    return evaluate(board) == 1; // 1 означає виграш AI
}

Pair makeAIMove() {
    Pair bestMove = findBestMove(board); // Знаходимо найкращий хід
    board[bestMove.first][bestMove.second] = 'O'; // AI робить хід за O
    moveCount++; // Збільшуємо лічильник ходів
    return bestMove;
}

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


void sendCurrentBoardState() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            Serial.print(board[i][j]); // Відправка символу
        }
    }
    Serial.println(); // Перехід на новий рядок
}

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

void BlueblinkLED() {
    if (blueLedState) { // Виконуємо тільки якщо діод увімкнений у налаштуваннях
        digitalWrite(BlueledPin, HIGH);
        delay(300);
        digitalWrite(BlueledPin, LOW);
    }
}

void YellowblinkLED() {
    if (yellowLedState) { // Виконуємо тільки якщо діод увімкнений у налаштуваннях
        digitalWrite(YellowledPin, HIGH);
        delay(300);
        digitalWrite(YellowledPin, LOW);
    }
}

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
//                                                                        **** AI LOGIC ****

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

bool isMovesLeft(char board[3][3]) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] == ' ') return false;
    return true;
}

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
