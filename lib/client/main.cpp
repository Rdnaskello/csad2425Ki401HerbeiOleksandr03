/**
 * @file main.cpp
 * @brief Tic-Tac-Toe game with Arduino backend and SFML frontend.
 */

#include <SFML/Graphics.hpp>
#include <iostream>
#include <windows.h> // Для роботи з серійним портом на Windows
#include "D:/simpleini-master/simpleini-master/SimpleIni.h"
/// Size of the game board (3x3).
const int SIZE_BOARD = 3;
/// Size of each tile in pixels.
const int TILE_SIZE = 100; 
/// Game board representation.
char board[SIZE_BOARD][SIZE_BOARD] = { {' ', ' ', ' '}, {' ', ' ', ' '}, {' ', ' ', ' '} }; 


HANDLE hSerial; ///< Handle for the serial port.
DCB dcbSerialParams = { 0 }; ///< Serial port configuration parameters.
COMMTIMEOUTS timeouts = { 0 }; ///< Serial port timeouts configuration.

/**
 * @brief Opens the serial port with specified configurations.
 * @param portName Name of the serial port (e.g., "COM7").
 * @return True if the port is opened successfully, false otherwise.
 */

bool openSerialPort(const char* portName) {
    hSerial = CreateFileA(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        return false;
    }

    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        return false;
    }

    dcbSerialParams.BaudRate = CBR_4800;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if (!SetCommState(hSerial, &dcbSerialParams)) {
        return false;
    }

    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (!SetCommTimeouts(hSerial, &timeouts)) {
        return false;
    }

    return true;
}

/**
 * @brief Updates the game board based on the response received from the Arduino.
 * @param response Response received from the Arduino.
 */

void updateBoardFromSerial(const std::string& response) {
    if (response.length() < SIZE_BOARD * SIZE_BOARD) {
        return; // Вийти з функції, якщо недостатньо даних
    }

    int index = 0;
    for (int i = 0; i < SIZE_BOARD; ++i) {
        for (int j = 0; j < SIZE_BOARD; ++j) {
            board[i][j] = response[index++];
        }
    }
}

/**
 * @brief Structure for storing game statistics.
 */

struct Stats {
    // PvP
    int pvpGames = 0; ///< Number of PvP games played.
    int winsX = 0; ///< Number of wins for player X.
    int lossesX = 0; ///< Number of losses for player X.
    int drawsX = 0; ///< Number of draws for player X.
    int winsO = 0; ///< Number of wins for player O.
    int lossesO = 0; ///< Number of losses for player O.
    int drawsO = 0; ///< Number of draws for player O.

    // AI Player First
    int Games = 0; ///< Number of games played.
    int Wins = 0;  ///< Number of wins.
    int Draws = 0;  ///< Number of draws.
    int Losses = 0; ///< Number of losses.
    int Winrate = 0;    ///< Winrate in percentage.

};

Stats stats; ///< Game statistics.

/**
 * @brief Saves the game statistics to an existing INI file.
 * @param filename Name of the INI file.
 */

void saveStatsToExistingINI(const std::string& filename) {
    CSimpleIniA ini;
    ini.SetUnicode();

    // Завантажуємо існуючий INI-файл
    if (ini.LoadFile(filename.c_str()) != SI_OK) {
        std::cout << "Failed to load INI file." << std::endl;
        return;
    }

    // Зберігаємо статистику PvP
    ini.SetLongValue("Stats_PvP", "Games", stats.pvpGames);
    ini.SetLongValue("Stats_PvP", "WinsX", stats.winsX);
    ini.SetLongValue("Stats_PvP", "LossesX", stats.lossesX);
    ini.SetLongValue("Stats_PvP", "DrawsX", stats.drawsX);
    ini.SetLongValue("Stats_PvP", "WinsO", stats.winsO);
    ini.SetLongValue("Stats_PvP", "LossesO", stats.lossesO);
    ini.SetLongValue("Stats_PvP", "DrawsO", stats.drawsO);

    // Зберігаємо статистику для AI-гри (гравець перший)
    ini.SetLongValue("Stats ", "Games", stats.Games);
    ini.SetLongValue("Stats ", "Wins", stats.Wins);
    ini.SetLongValue("Stats ", "Draws", stats.Draws);
    ini.SetLongValue("Stats ", "Losses", stats.Losses);
    ini.SetLongValue("Stats ", "Winrate", stats.Winrate);


    // Зберігаємо зміни в INI-файл
    if (ini.SaveFile(filename.c_str()) != SI_OK) {
        std::cout << "Failed to save INI file." << std::endl;
    }
    else {
        std::cout << "Stats saved to existing INI file successfully." << std::endl;
    }
}

/**
 * @brief Loads the game statistics from an existing INI file.
 * @param filename Name of the INI file.
 */

void loadStatsFromExistingINI(const std::string& filename) {
    CSimpleIniA ini;
    ini.SetUnicode();

    // Завантажуємо існуючий INI-файл
    if (ini.LoadFile(filename.c_str()) != SI_OK) {
        std::cout << "Failed to load INI file." << std::endl;
        return;
    }

    // Завантажуємо статистику PvP
    stats.pvpGames = ini.GetLongValue("Stats_PvP", "Games", 0);
    stats.winsX = ini.GetLongValue("Stats_PvP", "WinsX", 0);
    stats.lossesX = ini.GetLongValue("Stats_PvP", "LossesX", 0);
    stats.drawsX = ini.GetLongValue("Stats_PvP", "DrawsX", 0);
    stats.winsO = ini.GetLongValue("Stats_PvP", "WinsO", 0);
    stats.lossesO = ini.GetLongValue("Stats_PvP", "LossesO", 0);
    stats.drawsO = ini.GetLongValue("Stats_PvP", "DrawsO", 0);

    // Завантажуємо статистику для AI-гри (гравець перший)
    stats.Games = ini.GetLongValue("Stats ", "Games", 0);
    stats.Wins = ini.GetLongValue("Stats ", "Wins", 0);
    stats.Draws = ini.GetLongValue("Stats ", "Draws", 0);
    stats.Losses = ini.GetLongValue("Stats ", "Losses", 0);
    stats.Winrate = ini.GetLongValue("Stats ", "Winrate", 0);


    std::cout << "Stats loaded from existing INI file successfully." << std::endl;
}

/**
 * @brief Loads the configuration from an INI file.
 * @param filename Name of the INI file.
 * @param blueLedState State of the blue LED.
 * @param yellowLedState State of the yellow LED.
 */

void loadConfig(const std::string& filename, bool& blueLedState, bool& yellowLedState) {
    CSimpleIniA ini;
    ini.SetUnicode();
    if (ini.LoadFile(filename.c_str()) != SI_OK) {
        std::cout << "Failed to load INI file." << std::endl;
        return;
    }

    // Завантажуємо стан діодів
    blueLedState = ini.GetBoolValue("LEDs", "Blue", false);
    yellowLedState = ini.GetBoolValue("LEDs", "Yellow", false);

    // Лог станів
    std::cout << "Blue LED: " << (blueLedState ? "ON" : "OFF") << std::endl;
    std::cout << "Yellow LED: " << (yellowLedState ? "ON" : "OFF") << std::endl;
}

/**
 * @brief Saves LED configuration to an INI file.
 * @param filename Path to the INI file.
 * @param blueLedState Current state of the blue LED.
 * @param yellowLedState Current state of the yellow LED.
 */

void saveConfig(const std::string& filename, bool& blueLedState, bool& yellowLedState) {
    CSimpleIniA ini;
    ini.SetUnicode();
    if (ini.LoadFile(filename.c_str()) != SI_OK) {
        std::cout << "Failed to load INI file." << std::endl; // Замінено Serial.println
        return;
    }

    // Записуємо стан діодів
    ini.SetBoolValue("LEDs", "Blue", blueLedState);
    ini.SetBoolValue("LEDs", "Yellow", yellowLedState);

    // Зберігаємо зміни
    if (ini.SaveFile(filename.c_str()) != SI_OK) {
        std::cout << "Failed to save INI file." << std::endl; // Замінено Serial.println
    }
    else {
        std::cout << "Configuration saved successfully." << std::endl; // Лог успіху
    }
}

/**
 * @brief Clears the serial buffer by reading all available data.
 */
void clearSerialBuffer() {
    char buffer[256];
    DWORD bytes_read;
    while (ReadFile(hSerial, buffer, sizeof(buffer), &bytes_read, NULL) && bytes_read > 0) {
        // Просто зчитуємо всі дані в буфер, нічого не роблячи
    }
}


/**
 * @brief Writes data to the serial port.
 * @param data String to send to the Arduino.
 */
void writeSerialPort(const std::string& data) {

    DWORD bytes_written;
    if (WriteFile(hSerial, data.c_str(), data.size(), &bytes_written, NULL)) {
        std::cout << "[Frontend] Sent to Arduino: " << data << std::endl; // Лог даних, які відправляються
    }
    else {
        std::cout << "[Frontend] Error sending to Arduino!" << std::endl; // Лог помилки
    }

}

/**
 * @brief Reads data from the serial port.
 * @return String received from the Arduino.
 */

std::string readSerialPort() {
    char buffer[256] = { 0 }; // Ініціалізуйте буфер нулями
    DWORD bytes_read;
    if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytes_read, NULL)) {
        buffer[bytes_read] = '\0'; // Додайте термінальний нуль
        std::cout << "[Backend] Received from Arduino: " << buffer << std::endl; // Лог отриманих даних
        return std::string(buffer);

    }
    std::cout << "[Frontend] Error reading from Arduino!" << std::endl; // Лог помилки
    return ""; // Повертаємо пустий рядок у разі невдачі
}

/**
 * @brief Draws the Tic-Tac-Toe board grid.
 * @param window Reference to the SFML window.
 */
void drawBoard(sf::RenderWindow& window) {
    for (int i = 0; i <= SIZE_BOARD; ++i) {
        // Горизонтальні лінії
        sf::RectangleShape horizontalLine(sf::Vector2f(TILE_SIZE * SIZE_BOARD, 5));
        horizontalLine.setPosition(0, i * TILE_SIZE);
        horizontalLine.setFillColor(sf::Color::Black);
        window.draw(horizontalLine);

        // Вертикальні лінії
        sf::RectangleShape verticalLine(sf::Vector2f(5, TILE_SIZE * SIZE_BOARD));
        verticalLine.setPosition(i * TILE_SIZE - 4, 0); // Додаємо поправку
        verticalLine.setFillColor(sf::Color::Black);
        window.draw(verticalLine);
    }
}


/**
 * @brief Draws X and O marks on the game board.
 * @param window Reference to the SFML window.
 * @param font Font used for the marks.
 */
void drawMarks(sf::RenderWindow& window, sf::Font& font) {
    for (int i = 0; i < SIZE_BOARD; ++i) {
        for (int j = 0; j < SIZE_BOARD; ++j) {
            if (board[i][j] != ' ') {
                sf::Text text;
                text.setFont(font);
                text.setString(board[i][j]);
                text.setCharacterSize(100);
                text.setPosition(j * TILE_SIZE + 15, i * TILE_SIZE - 20);
                text.setFillColor(sf::Color::Black);
                window.draw(text);
            }
        }
    }
}

/**
 * @brief Resets the game board to its initial state.
 */
void resetBoard() {
    for (int i = 0; i < SIZE_BOARD; ++i) {
        for (int j = 0; j < SIZE_BOARD; ++j) {
            board[i][j] = ' '; // Очищуємо дошку
        }
    }
}

/**
 * @brief Draws the game interface including board, marks, and buttons.
 * @param window Reference to the SFML window.
 * @param font Font used for button texts.
 * @param playerFirstButton Rectangle shape for the "Player First" button.
 * @param playerFirstText Text displayed on the "Player First" button.
 * @param aiFirstButton Rectangle shape for the "AI First" button.
 * @param aiFirstText Text displayed on the "AI First" button.
 * @param restartButton Rectangle shape for the "Restart" button.
 * @param restartText Text displayed on the "Restart" button.
 * @param pvpButton Rectangle shape for the "PvP" button.
 * @param pvpText Text displayed on the "PvP" button.
 * @param settingsButton Rectangle shape for the "Settings" button.
 * @param settingsText Text displayed on the "Settings" button.
 */
void drawGame(sf::RenderWindow& window, sf::Font& font, sf::RectangleShape playerFirstButton, sf::Text playerFirstText, sf::RectangleShape aiFirstButton, sf::Text aiFirstText, sf::RectangleShape restartButton, sf::Text restartText, sf::RectangleShape pvpButton, sf::Text pvpText, sf::RectangleShape settingsButton, sf::Text settingsText) {
	window.clear(sf::Color::White);   
	drawBoard(window);                // Малюємо дошку
	drawMarks(window, font);          // Малюємо мітки (хрестики і нулики)
    window.draw(playerFirstButton);   // Малюємо кнопку вибору черговості
    window.draw(playerFirstText);     // Текст на кнопці "Player First"
    window.draw(aiFirstButton);       // Малюємо кнопку вибору черговості
    window.draw(aiFirstText);         // Текст на кнопці "AI First"
    window.draw(restartButton);       // Малюємо кнопку рестарту
    window.draw(restartText);         // Малюємо текст на кнопці рестарту
    window.draw(pvpButton);           // Малюємо кнопку PvP
    window.draw(pvpText);             // Малюємо текст на кнопці PvP
    window.draw(settingsButton);       // Малюємо кнопку налаштувань
    window.draw(settingsText);         // Малюємо текст на кнопці налаштувань
    window.display();                 // Відображаємо все це у вікні

}

/**
 * @brief Draws the game interface including board, marks, and buttons.
 * @param window Reference to the SFML window.
 * @param font Font used for button texts.
 * @param playerFirstButton Rectangle shape for the "Player First" button.
 * @param playerFirstText Text displayed on the "Player First" button.
 * @param aiFirstButton Rectangle shape for the "AI First" button.
 * @param aiFirstText Text displayed on the "AI First" button.
 * @param restartButton Rectangle shape for the "Restart" button.
 * @param restartText Text displayed on the "Restart" button.
 * @param pvpButton Rectangle shape for the "PvP" button.
 * @param pvpText Text displayed on the "PvP" button.
 * @param settingsButton Rectangle shape for the "Settings" button.
 * @param settingsText Text displayed on the "Settings" button.
 */
void drawSettingsMenu(sf::RenderWindow& settingsWindow, sf::Font& font, sf::RectangleShape& blueLedButton, sf::Text& blueLedText, sf::RectangleShape& yellowLedButton, sf::Text& yellowLedText) {
    settingsWindow.clear(sf::Color::White);

    // Кнопка для керування синім діодом
    settingsWindow.draw(blueLedButton);
    settingsWindow.draw(blueLedText);

    // Кнопка для керування жовтим діодом
    settingsWindow.draw(yellowLedButton);
    settingsWindow.draw(yellowLedText);

    settingsWindow.display();
}

/**
 * @brief Opens the settings menu for controlling LED states.
 * @param font Font used for the settings menu.
 * @param blueLedState Reference to the state of the blue LED.
 * @param yellowLedState Reference to the state of the yellow LED.
 */
void openSettingsMenu(sf::Font& font, bool& blueLedState, bool& yellowLedState) {
    sf::RenderWindow settingsWindow(sf::VideoMode(400, 300), "Settings");

    // Кнопка для керування синім діодом
    sf::RectangleShape blueLedButton(sf::Vector2f(200, 50));
    blueLedButton.setPosition(100, 50);
    blueLedButton.setFillColor(sf::Color::Blue);

    sf::Text blueLedText;
    blueLedText.setFont(font);
    blueLedText.setCharacterSize(20);
    blueLedText.setFillColor(sf::Color::White);

    // Кнопка для керування жовтим діодом
    sf::RectangleShape yellowLedButton(sf::Vector2f(200, 50));
    yellowLedButton.setPosition(100, 150);
    yellowLedButton.setFillColor(sf::Color::Yellow);

    sf::Text yellowLedText;
    yellowLedText.setFont(font);
    yellowLedText.setCharacterSize(20);
    yellowLedText.setFillColor(sf::Color::Black);

    while (settingsWindow.isOpen()) {
        sf::Event event;
        while (settingsWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                settingsWindow.close();
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                int mouseX = event.mouseButton.x;
                int mouseY = event.mouseButton.y;

                if (blueLedButton.getGlobalBounds().contains(mouseX, mouseY)) {
                    blueLedState = !blueLedState; // Змінюємо стан
                    saveConfig("D:/scad/csad2425Ki401HerbeiOleksandr03/config/config.ini", blueLedState, yellowLedState);
                    writeSerialPort("BLed\n");
                }
                else if (yellowLedButton.getGlobalBounds().contains(mouseX, mouseY)) {
                    yellowLedState = !yellowLedState; // Змінюємо стан
                    saveConfig("D:/scad/csad2425Ki401HerbeiOleksandr03/config/config.ini", blueLedState, yellowLedState);
                    writeSerialPort("Yled\n");
                }
            }
        }

        // Оновлюємо текст кнопок залежно від стану діодів
        blueLedText.setString(blueLedState ? "Blue LED: ON" : "Blue LED: OFF");
        blueLedText.setPosition(blueLedButton.getPosition().x + 20, blueLedButton.getPosition().y + 10);

        yellowLedText.setString(yellowLedState ? "Yellow LED: ON" : "Yellow LED: OFF");
        yellowLedText.setPosition(yellowLedButton.getPosition().x + 20, yellowLedButton.getPosition().y + 10);

        drawSettingsMenu(settingsWindow, font, blueLedButton, blueLedText, yellowLedButton, yellowLedText);
    }
}



/**
 * @brief Main function for the Tic-Tac-Toe game.
 * Initializes the game window, serial communication, and event handling.
 * @return Exit status of the program (0 for success).
 */
int main() {
    sf::RenderWindow window(sf::VideoMode(TILE_SIZE * SIZE_BOARD, TILE_SIZE * SIZE_BOARD + 200), "Tic-Tac-Toe with Arduino Backend");
    sf::Font font;
    if (!font.loadFromFile("C:/Windows/Fonts/Arial.ttf")) {
        return 1;
    }

    if (!openSerialPort("COM7")) {
        return 1;
    }

    bool blueLedState;
    bool yellowLedState;

    bool gameOver = false;
    bool resetRequested = false; // Додаємо змінну для фіксації запиту на скидання
    loadConfig("D:/scad/csad2425Ki401HerbeiOleksandr03/config/config.ini", blueLedState, yellowLedState);
    loadStatsFromExistingINI("D:/scad/csad2425Ki401HerbeiOleksandr03/config/config.ini");
    // Створення кнопок для вибору черговості
    sf::RectangleShape playerFirstButton(sf::Vector2f(150, 50)); // Кнопка вибору черговості гравця
    playerFirstButton.setPosition((TILE_SIZE * SIZE_BOARD - 300) / 2, TILE_SIZE * SIZE_BOARD + 20);
    playerFirstButton.setFillColor(sf::Color::Blue);
    sf::Text playerFirstText;
    playerFirstText.setFont(font);
    playerFirstText.setString("Player First");
    playerFirstText.setCharacterSize(24);
    playerFirstText.setFillColor(sf::Color::White);
    playerFirstText.setPosition(playerFirstButton.getPosition().x + 10, playerFirstButton.getPosition().y + 10);

    sf::RectangleShape aiFirstButton(sf::Vector2f(150, 50)); // Кнопка вибору черговості AI
    aiFirstButton.setPosition((TILE_SIZE * SIZE_BOARD + 50) / 2, TILE_SIZE * SIZE_BOARD + 20);
    aiFirstButton.setFillColor(sf::Color::Red);
    sf::Text aiFirstText;
    aiFirstText.setFont(font);
    aiFirstText.setString("AI First");
    aiFirstText.setCharacterSize(24);
    aiFirstText.setFillColor(sf::Color::White);
    aiFirstText.setPosition(aiFirstButton.getPosition().x + 10, aiFirstButton.getPosition().y + 10);

    sf::RectangleShape restartButton(sf::Vector2f(150, 50)); // Кнопка перезавантаження
    restartButton.setPosition(aiFirstButton.getPosition().x, aiFirstButton.getPosition().y + 60); // Позиція праворуч під AI
    restartButton.setFillColor(sf::Color::Green);
    sf::Text restartText;
    restartText.setFont(font);
    restartText.setString("Restart");
    restartText.setCharacterSize(24);
    restartText.setFillColor(sf::Color::White);
    restartText.setPosition(restartButton.getPosition().x + 10, restartButton.getPosition().y + 10);

    sf::RectangleShape pvpButton(sf::Vector2f(150, 50)); // Кнопка PvP
    pvpButton.setPosition(playerFirstButton.getPosition().x, playerFirstButton.getPosition().y + 60); // Позиція під Player First
    pvpButton.setFillColor(sf::Color::Yellow);
    sf::Text pvpText;
    pvpText.setFont(font);
    pvpText.setString("PvP");
    pvpText.setCharacterSize(24);
    pvpText.setFillColor(sf::Color::Black);
    pvpText.setPosition(pvpButton.getPosition().x + 10, pvpButton.getPosition().y + 10);

    // кнопка для налаштування
    sf::RectangleShape settingsButton(sf::Vector2f(150, 50)); // Кнопка налаштувань
    settingsButton.setPosition(pvpButton.getPosition().x, pvpButton.getPosition().y + 60); // Позиція під PvP
    settingsButton.setFillColor(sf::Color::Magenta);
    sf::Text settingsText;
    settingsText.setFont(font);
    settingsText.setString("Settings");
    settingsText.setCharacterSize(24);
    settingsText.setFillColor(sf::Color::Black);
    settingsText.setPosition(settingsButton.getPosition().x + 10, settingsButton.getPosition().y + 10);
            
    // Initialization of game buttons and their positions...
    // (This section is already documented in function parameters above.)
    drawGame(window,font, playerFirstButton, playerFirstText, aiFirstButton, aiFirstText, restartButton, restartText, pvpButton, pvpText, settingsButton, settingsText);
    


    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                int mouseX = event.mouseButton.x;
                int mouseY = event.mouseButton.y;

                if (restartButton.getGlobalBounds().contains(mouseX, mouseY)) {
                    writeSerialPort("reset\n");
                    resetRequested = true; // Запит на скидання без очищення дошки
                }
                else if (playerFirstButton.getGlobalBounds().contains(mouseX, mouseY)) {
                    writeSerialPort("player\n");
                    resetBoard(); // Очистити дошку
                    resetRequested = true; // Запит на скидання з очищенням дошки
                }
                else if (aiFirstButton.getGlobalBounds().contains(mouseX, mouseY)) {
                    writeSerialPort("ai\n");
                    resetBoard(); // Очистити дошку
                    resetRequested = true; // Запит на скидання з очищенням дошки
                }
                else if (settingsButton.getGlobalBounds().contains(mouseX, mouseY)) {
                    openSettingsMenu(font, yellowLedState, blueLedState);
                }
                else if (pvpButton.getGlobalBounds().contains(mouseX, mouseY)) {
                    writeSerialPort("pvp\n");
                    resetBoard(); // Очистити дошку
                    resetRequested = true; // Запит на скидання з очищенням дошки
                }
                else if (!gameOver && !resetRequested) { // Не обробляємо хід, якщо очікуємо на скидання
                    int row = mouseY / TILE_SIZE;
                    int col = mouseX / TILE_SIZE;

                    if (row < SIZE_BOARD && col < SIZE_BOARD && board[row][col] == ' ') {
                        std::string move = std::to_string(row) + "," + std::to_string(col) + "\n";
                        writeSerialPort(move);
                        
                        // Дочекайтеся відповіді від Arduino
                        std::string response = readSerialPort();
                        updateBoardFromSerial(response);

                        // Обробка результату гри
                        if (response.find("X win!") != std::string::npos) {
                            stats.winsX++;
                            stats.lossesO++;
                            stats.pvpGames++;
                            
                            gameOver = true;
                        }
                        else if (response.find("O win!") != std::string::npos) {
                            stats.winsO++;
                            stats.lossesX++;
                            stats.pvpGames++;
                            gameOver = true;
                        }
                        else if (response.find("AI win!") != std::string::npos) {
                            stats.Losses++;
                            stats.Games++;
                            stats.Winrate = (stats.Wins / stats.Games) * 100;
                            gameOver = true;
                        }
                        else if (response.find("You win!") != std::string::npos) {
                            stats.Wins++;
                            stats.Games++;
                            stats.Winrate = (stats.Wins / stats.Games) * 100;
                            gameOver = true;
                        }
                        else if (response.find("Draw!") != std::string::npos) {
                            stats.drawsX++;
                            stats.drawsO++;
                            stats.Draws++;
                            stats.Games++;
                            gameOver = true;
                        }

                        // Зберігаємо статистику після завершення гри
                        if (gameOver) {
                            saveStatsToExistingINI("D:/scad/csad2425Ki401HerbeiOleksandr03/config/config.ini");
                        }

                        // Оновити відображення дошки після кожного ходу
                        drawGame(window, font, playerFirstButton, playerFirstText, aiFirstButton, aiFirstText, restartButton, restartText, pvpButton, pvpText, settingsButton, settingsText);
                    }
                }
            }
        }

        // Перевірка на відповідь від Arduino після запиту скидання
        if (resetRequested) {
            std::string response = readSerialPort();
            if (!response.empty()) {
                updateBoardFromSerial(response);
                resetRequested = false; // Завершили скидання
                gameOver = false;       // Гра триває
                drawGame(window, font, playerFirstButton, playerFirstText, aiFirstButton, aiFirstText, restartButton, restartText, pvpButton, pvpText, settingsButton, settingsText);
            }
        }
    }


    CloseHandle(hSerial); // Закрити серійний порт після виходу
    return 0;
}