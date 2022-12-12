CC = g++
CFLAGS = -g -Wall -std=c++17 -lz -lbz2 -larchive

TARGET = main

all: $(TARGET)

$(TARGET): $(TARGET).cpp
		$(CC) $(CFLAGS) $(TARGET).cpp -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
		$(RM) $(TARGET)
		$(RM) a.out
		$(RM) -rf .vscode
		$(RM) -rf $(TARGET).dSYM