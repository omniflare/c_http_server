CC = gcc
CFLAGS = -Wall -Wextra
TARGET = httpd
SRCS = httpd.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean dirs

all: dirs $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

dirs:
	@mkdir -p app img

clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf app/* img/*

install:
	cp $(TARGET) /usr/local/bin/

# Development helpers
run: $(TARGET)
	./$(TARGET) 8080

test: all
	@echo "Testing server on port 8080..."
	./$(TARGET) 8080 & \
	sleep 1 && \
	curl http://localhost:8080/app/index.html || true && \
	pkill $(TARGET)