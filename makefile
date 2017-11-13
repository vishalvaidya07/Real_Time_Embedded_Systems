CC = gcc
CFLAGS = -g -Wall
LM = -lm
TARGET = Assignment_3
run: $(TARGET)
$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(LM)
