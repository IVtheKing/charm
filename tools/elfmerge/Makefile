BIN=build/elfmerge
OBJ=build/elfmerge.o
SRC=elfmerge.c

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ)

build/%.o: %.c
	@test -d $(dir $@) || mkdir -pm 775 $(dir $@)
	$(CC) -g -D__APPLE__ -c -o $@ $<

clean:
	rm -rf build/

