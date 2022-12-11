TARGET = BJServer
PORT = 47200

all: $(TARGET)

$(TARGET): $(TARGET).c
	gcc $(TARGET).c -o $(TARGET) -lpthread

run: $(TARGET)
	./$(TARGET) $(PORT)

clean:
	$(RM) $(TARGET)
