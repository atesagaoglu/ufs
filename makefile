CC = gcc
CFLAGS = -std=gnu11 -Wall -Wextra -pedantic

BUILD = build

SRC = \
	src/main.c \
	src/log.c

OBJ = $(patsubst src/%.c, $(BUILD)/%.o, $(SRC))

TARGET = ufs

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

$(BUILD)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

clean:
	rm -rf $(BUILD) $(TARGET)

-include $(OBJ:.o=.d)
