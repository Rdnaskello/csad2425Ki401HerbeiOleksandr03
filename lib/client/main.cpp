#include <iostream>
#include <windows.h> // Для роботи із серійним портом

HANDLE hSerial; // Дескриптор серійного порту

// Функція для відкриття серійного порту
bool openSerialPort(const char* portName) {
    hSerial = CreateFileA(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: Unable to open serial port!" << std::endl;
        return false;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error: Unable to get serial port state!" << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    dcbSerialParams.BaudRate = CBR_9600; // Швидкість 4800 бод
    dcbSerialParams.ByteSize = 8;       // 8 біт даних
    dcbSerialParams.StopBits = ONESTOPBIT; // 1 стоп-біт
    dcbSerialParams.Parity = NOPARITY;     // Без парності
    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error: Unable to set serial port state!" << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        std::cerr << "Error: Unable to set timeouts!" << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    return true;
}

// Функція для запису в серійний порт
void writeSerialPort(const std::string& data) {
    DWORD bytesWritten;
    if (!WriteFile(hSerial, data.c_str(), data.size(), &bytesWritten, NULL)) {
        std::cerr << "Error: Failed to write to serial port!" << std::endl;
    } else {
        std::cout << "Sent to Arduino: " << data << std::endl;
    }
}

// Функція для читання з серійного порту
std::string readSerialPort() {
    char buffer[256] = { 0 }; // Буфер для отримання даних
    DWORD bytesRead;
    if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0'; // Додаємо завершальний символ
        return std::string(buffer);
    } else {
        return ""; // Повертаємо порожній рядок у разі невдачі
    }
}

// Основна функція
int main() {
    const char* portName = "COM7"; // Змініть на ваш COM порт

    // Відкриваємо серійний порт
    if (!openSerialPort(portName)) {
        return 1;
    }

    while (true) {
        // Відправляємо повідомлення на сервер
        std::string message = "Hmm... I'll make this move.\n";
        writeSerialPort(message);

        // Читаємо відповідь від сервера
        std::string response = readSerialPort();
        if (!response.empty()) {
            std::cout << "Server says: " << response << std::endl;
        }

        // Перевірка на завершення
        std::cout << "Do you want to send another message? (y/n): ";
        char choice;
        std::cin >> choice;
        if (choice == 'n' || choice == 'N') {
            break;
        }
    }

    // Закриваємо серійний порт
    CloseHandle(hSerial);
    return 0;
}
