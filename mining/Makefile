CC = g++
CFLAGS = -g -Wall -std=c++17 -O3 -lz -lbz2 -larchive -I. -O2 -pthread

TARGET = miner

all: $(TARGET)

miner: $(TARGET).cpp
		$(CC) $(CFLAGS) $(TARGET).cpp -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
		$(RM) $(TARGET)
		$(RM) a.out
		$(RM) -rf .vscode
		$(RM) -rf $(TARGET).dSYM