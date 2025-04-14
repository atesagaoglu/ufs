SRC = \
	  main.c

OBJ = $(SRC:.c=.o)

TARGET = ufs

all: $(TARGET)

$(TARGET): $(OBJ)
	gcc -o $(TARGET) $(OBJ)

%.o: %.c
	gcc -c -o $@ $<


clean:
	rm -f $(OBJ) $(TARGET)
