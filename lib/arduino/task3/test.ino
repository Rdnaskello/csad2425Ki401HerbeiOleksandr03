#include <AUnit.h>
#include <EEPROM.h>

test(EvaluateFunctionTest) {
    char testBoard1[3][3] = {
        {'X', 'X', 'X'},
        {' ', 'O', ' '},
        {' ', ' ', 'O'}
    };
    assertEqual(evaluate(testBoard1), -1); 

    char testBoard2[3][3] = {
        {'O', 'O', 'O'},
        {' ', 'X', ' '},
        {' ', ' ', 'X'}
    };
    assertEqual(evaluate(testBoard2), 1);

    char testBoard3[3][3] = {
        {'X', 'O', 'X'},
        {'O', 'O', 'X'},
        {'X', 'X', 'O'}
    };
    assertEqual(evaluate(testBoard3), 0); 
}
test(ResetBoardTest) {
    resetBoard();
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            assertEqual(board[i][j], ' ');
        }
    }
    assertEqual(moveCount, 0);
    assertFalse(gameOver); 
}
test(AIMoveTest) {
    Serial.println("BlockingMove...");
    resetBoard();
    board[0][0] = 'X'; 
    board[1][1] = 'X';
    board[1][2] = 'O';

    makeAIMove(); 
    assertEqual(board[2][2], 'O'); 

    Serial.println("ForkMove...");
    resetBoard();
    board[1][0] = 'X'; 
    board[1][1] = 'O';
    board[0][0] = 'O';
    board[2][2] = 'X';

    makeAIMove();
    assertEqual(board[0][1], 'O'); 

    Serial.println("ForkMove2...");
    resetBoard();
    board[1][1] = 'O'; 
    board[1][2] = 'X';
    board[2][2] = 'O';
    board[0][0] = 'X';

    makeAIMove(); 
    assertEqual(board[2][0], 'O'); 

    Serial.println("WinningMove...");
    resetBoard();
    board[0][0] = 'X'; 
    board[1][1] = 'O';
    board[2][2] = 'X';
    board[0][1] = 'O';

    makeAIMove(); 
    assertEqual(board[2][1], 'O'); 

    Serial.println("Best pos...");
    resetBoard();
    makeAIMove(); 
    assertEqual(board[1][1], 'O'); 
}

test(ProcessCommandTest) {
    strcpy(receivedData, "reset");
    processCommand();
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            assertEqual(board[i][j], ' '); 
        }
    }
    strcpy(receivedData, "BLed");
    processCommand();
    assertTrue(blueLedState); 
}

test(MinimaxBlockTest) {
    char testBoard[3][3] = {
        {'X', 'X', ' '},
        {'O', ' ', ' '},
        {' ', ' ', ' '}
    };
    int score = minimax(testBoard, 0, true, -1000, 1000); 
    assertEqual(score, 0); 
}

test(MinimaxDrawTest) {
    char testBoard[3][3] = {
        {'O', 'X', 'O'},
        {'X', 'O', 'X'},
        {'X', 'O', ' '}
    };
    int score = minimax(testBoard, 0, true, -1000, 1000); 
    assertEqual(score, 0); 
}

