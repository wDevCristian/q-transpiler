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

builgen: ./gen-code/1.c
	gcc $< -o $@

clean: 
	rm -vf $(OBJ) 1.c gen-code/1.c build builgen

all: 
	@echo $(PREF_SRC)
	@echo $(PREF_OBJ)
	@echo $(SRC)
	@echo $(OBJ)

# alias bld='make clean; make'
# alias exec-bld='./build > logger.md; make builgen; ./builgen'