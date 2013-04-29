CC:=gcc

BIN:=build/elfmerge
OBJ:=build/elfmerge.o
SRC:=elfmerge.c

ROOT_DIR	:=	$(realpath ../..)

INCLUDES=$(ROOT_DIR)/includes $(ROOT_DIR)/includes/util

INCLUDES	:=	$(addprefix -I ,$(INCLUDES))
CFLAGS		:=	-Wall -D DESKTOP_TARGET

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ)

build/%.o: %.c
	@test -d $(dir $@) || mkdir -pm 775 $(dir $@)
	$(CC) -g -D__APPLE__ -c $(INCLUDES) $(CFLAGS) -o $@ $<

clean:
	rm -rf build/