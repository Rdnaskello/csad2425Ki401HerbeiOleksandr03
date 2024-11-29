# Шлях до бібліотек і файлів
INCLUDES = -I"D:/libs/SFML-2.6.2/include" -I"D:/simpleini-master/simpleini-master"
LIBS = -L"D:/libs/SFML-2.6.2/lib" -lsfml-graphics -lsfml-window -lsfml-system

# Компілятор і параметри
CXX = g++
CXXFLAGS = -Wall -g $(INCLUDES)

# Файли проєкту
SRC = lib/client/main.cpp
OBJ = main.o
TARGET = lib/client/client.exe

# Правило за замовчуванням
all: $(TARGET)

# Як створити ціль
$(TARGET): $(SRC)
	$(CXX) $(SRC) $(CXXFLAGS) $(LIBS) -o $(TARGET)

# Очистка
clean:
	del /Q *.o
	del /Q $(TARGET)
