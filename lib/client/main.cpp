#include <iostream>
#include <fstream>
#include <string>
#include <windows.h> // Для роботи із серійним портом
#include <cstdlib>   // Для getenv

HANDLE hSerial; // Дескриптор серійного порту
std::ofstream mockSerialOut; // Файл для емуляції запису
std::ifstream mockSerialIn;  // Файл для емуляції читання

// Перевіряємо, чи запускається програма в CI-середовищі
bool isCIEnvironment() {
    const char* ci = std::getenv("CI");
    return ci != nullptr;
}

// Функція для відкриття серійного порту або файлу
bool openSerialPort(const char* portName) {
    if (isCIEnvironment()) {
        // У CI-середовищі використовуємо файл як "серійний порт"
        mockSerialOut.open("mock_serial_out.txt");
        mockSerialIn.open("mock_serial_in.txt");
        if (!mockSerialOut.is_open() || !mockSerialIn.is_open()) {
            std::cerr << "Error: Unable to open mock serial port files!" << std::endl;
            return false;
        }
        std::cout << "Mock serial port initialized for CI environment." << std::endl;
        return true;
    }

    // У звичайному середовищі працюємо з реальним серійним портом
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

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error: Unable to set serial port state!" << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    return true;
}

// Функція для запису в серійний порт або файл
void writeSerialPort(const std::string& data) {
    if (isCIEnvironment()) {
        if (mockSerialOut.is_open()) {
            mockSerialOut << data << std::endl;
            std::cout << "Mock Sent: " << data << std::endl;
        }
    } else {
        DWORD bytesWritten;
        if (!WriteFile(hSerial, data.c_str(), data.size(), &bytesWritten, NULL)) {
            std::cerr << "Error: Failed to write to serial port!" << std::endl;
        } else {
            std::cout << "Sent to Arduino: " << data << std::endl;
        }
    }
}

// Функція для читання із серійного порту або файлу
std::string readSerialPort() {
    if (isCIEnvironment()) {
        if (mockSerialIn.is_open()) {
            std::string line;
            if (std::getline(mockSerialIn, line)) {
                return line;
            }
        }
        return "";
    }

    char buffer[256] = { 0 };
    DWORD bytesRead;
    if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        return std::string(buffer);
    }
    return "";
}

// Основна функція
int main() {
    const char* portName = "COM7";

    if (!openSerialPort(portName)) {
        return 1;
    }

    while (true) {
        std::string message = "Hmm... I'll make this move.\n";
        writeSerialPort(message);

        std::string response = readSerialPort();
        if (!response.empty()) {
            std::cout << "Server says: " << response << std::endl;
        }

        std::cout << "Do you want to send another message? (y/n): ";
        char choice;
        std::cin >> choice;
        if (choice == 'n' || choice == 'N') {
            break;
        }
    }

    if (isCIEnvironment()) {
        mockSerialOut.close();
        mockSerialIn.close();
    } else {
        CloseHandle(hSerial);
    }
    return 0;
}
