CC = gcc
ARGS = -g

PREF_SRC = ./src/
PREF_OBJ = ./obj/

SRC = $(wildcard $(PREF_SRC)*.c)
OBJ = $(patsubst $(PREF_SRC)%.c, $(PREF_OBJ)%.o, $(SRC))

build: $(OBJ)
	$(CC) $(ARGS) $(OBJ) -o build 

$(PREF_OBJ)%.o: $(PREF_SRC)%.c
	$(CC) $(ARGS) -c $< -o $@

clean: 
	rm -vf $(OBJ) build

all: 
	@echo $(PREF_SRC)
	@echo $(PREF_OBJ)
	@echo $(SRC)
	@echo $(OBJ)