INTERMEDIATE_DIR?=stage
OUTPUT_DIR?=bin

SRC_DIRS=.
C_FILES=$(wildcard ${dir}/*.c)
C_SOURCES=$(foreach dir,${SRC_DIRS},${C_FILES})
C_OBJECTS=$(patsubst %.c,${INTERMEDIATE_DIR}/%.c.o,${C_SOURCES})
CPP_FILES=$(wildcard ${dir}/*.cpp)
CPP_SOURCES=$(foreach dir,${SRC_DIRS},${CPP_FILES})
CPP_OBJECTS=$(patsubst %.cpp,${INTERMEDIATE_DIR}/%.cpp.o,${CPP_SOURCES})
ASM_FILES=$(wildcard ${dir}/*.s)
ASM_SOURCES=$(foreach dir,${SRC_DIRS},${ASM_FILES})
ASM_OBJECTS=$(patsubst %.s,${INTERMEDIATE_DIR}/%.s.o,${ASM_SOURCES})
OBJECTS=${ASM_OBJECTS} ${C_OBJECTS} ${CPP_OBJECTS}
INTERMEDIATE_DIRS=$(foreach dir,${SRC_DIRS},${INTERMEDIATE_DIR}/${dir})

CC=gcc
CIGNORES=-Werror=unused-value
CFLAGS=-m64 -O0 -Wall -Wextra -Werror ${CIGNORES} -g -I../src -Itest -fpic

CPP=g++
CPPFLAGS=${CFLAGS} -std=c++11

LD=gcc
LDFLAGS_LIB=-shared
LDFLAGS_EXE=-pie -Wl,-rpath,.
LDLIBS_LIB=${OUTPUT_DIR}/libelf-hook.so -lgtest -lgmock -lstdc++ -pthread
LDLIBS_EXE=${OUTPUT_DIR}/libtest.so

AS=as
ASFLAGS=

${OUTPUT_DIR}/libtest.so: ${OBJECTS} ${OUTPUT_DIR} 
	${LD} ${LDFLAGS_LIB} -o $@ ${OBJECTS} ${LDLIBS_LIB}

${OUTPUT_DIR}/test: ${OUTPUT_DIR} ${OUTPUT_DIR}/libtest.so
	${LD} ${LDFLAGS_EXE} -o $@ ${LDLIBS_EXE}

${OUTPUT_DIR}:
	mkdir $@

${INTERMEDIATE_DIR}:
	mkdir $@

${OBJECTS}: | ${INTERMEDIATE_DIRS}

${INTERMEDIATE_DIRS}:
	@mkdir -p $@

${INTERMEDIATE_DIR}/%.c.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

${INTERMEDIATE_DIR}/%.cpp.o: %.cpp
	${CPP} ${CPPFLAGS} -c $< -o $@

${INTERMEDIATE_DIR}/%.s.o: %.s
	${AS} ${ASFLAGS} $< -o $@

all: ${OUTPUT_DIR}/test

clean:
	rm -f ${OUTPUT_DIR}/test
	rm -f ${OUTPUT_DIR}/libtest.so
	rm -f ${OBJECTS}

.PHONY: all clean