CFLAGS = -Wall -Wextra -Ikernel/include -Wcomment -Wno-unknown-pragmas -Wno-unused-result -Wno-empty-body -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-unused-variable -Wno-format-overflow
CC = gcc

# Logger flags
ERROR_LOGS ?= 1
WARN_LOGS ?= 1
INFO_LOGS ?= 1
DEBUG_LOGS ?= 1
IO_LOGS ?= 1
MEM_LOGS ?= 0
LOGGING_LOGS ?= 1
SPECIAL_LOGS ?= 1

########
# Logger flags
ifeq ($(ERROR_LOGS), 1)
    CFLAGS += -DERROR_LOGS
endif

ifeq ($(WARN_LOGS), 1)
    CFLAGS += -DWARNING_LOGS
endif

ifeq ($(INFO_LOGS), 1)
    CFLAGS += -DINFO_LOGS
endif

ifeq ($(DEBUG_LOGS), 1)
    CFLAGS += -DDEBUG_LOGS
endif

ifeq ($(IO_LOGS), 1)
    CFLAGS += -DIO_OPERATION_LOGS
endif

ifeq ($(MEM_LOGS), 1)
    CFLAGS += -DMEM_OPERATION_LOGS
endif

ifeq ($(LOGGING_LOGS), 1)
    CFLAGS += -DLOGGING_LOGS
endif

ifeq ($(SPECIAL_LOGS), 1)
    CFLAGS += -DSPECIAL_LOGS
endif

OUTPUT = builds/ccompiler
SOURCES = main.c src/*.c src/syntax/*.c src/optimization/*.c std/*.c

all: force_build $(OUTPUT)

force_build:
	@if [ -e $(OUTPUT) ]; then rm -f $(OUTPUT); fi

$(OUTPUT): $(SOURCES)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(SOURCES) -g -DPRINT_PARSE

clean:
	rm -f $(OUTPUT)

.PHONY: all clean force_build