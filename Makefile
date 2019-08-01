INTERMEDIATE_DIR?=stage
OUTPUT_DIR?=bin

SRC_DIRS=.
C_FILES=$(wildcard ${dir}/*.c)
C_SOURCES=$(foreach dir,${SRC_DIRS},${C_FILES})
C_OBJECTS=$(patsubst %.c,${INTERMEDIATE_DIR}/%.c.o,${C_SOURCES})
ASM_FILES=$(wildcard ${dir}/*.s)
ASM_SOURCES=$(foreach dir,${SRC_DIRS},${ASM_FILES})
ASM_OBJECTS=$(patsubst %.s,${INTERMEDIATE_DIR}/%.s.o,${ASM_SOURCES})
OBJECTS=${ASM_OBJECTS} ${C_OBJECTS}
INTERMEDIATE_DIRS=$(foreach dir,${SRC_DIRS},${INTERMEDIATE_DIR}/${dir})

CC=gcc
CC_FLAGS=$(CFLAGS) -m64 -std=gnu99 -O0 -Wall -Wextra -Werror -g -fpic

LD=gcc
LD_FLAGS=-shared

AR=ar
AR_FLAGS=rcs

AS=as
AS_FLAGS=

${OUTPUT_DIR}/libelf-hook.a: ${OBJECTS} ${OUTPUT_DIR}
	${AR} ${AS_FLAGS} -o $@ ${OBJECTS}

${OUTPUT_DIR}:
	mkdir $@

${INTERMEDIATE_DIR}:
	mkdir $@

${OBJECTS}: | ${INTERMEDIATE_DIRS}

${INTERMEDIATE_DIRS}:
	@mkdir -p $@

${INTERMEDIATE_DIR}/%.c.o: %.c
	${CC} ${CC_FLAGS} -c $< -o $@

${INTERMEDIATE_DIR}/%.s.o: %.s
	${AS} ${AS_FLAGS} $< -o $@

all: ${OUTPUT_DIR}/libelf-hook.a

clean:
	rm -f ${OUTPUT_DIR}/libelf-hook.a
	rm -f ${OBJECTS}

.PHONY: all clean
