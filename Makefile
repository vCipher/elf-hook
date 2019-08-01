CURRENT_DIR=$(shell pwd)
INTERMEDIATE_DIR=${CURRENT_DIR}/stage
OUTPUT_DIR=${CURRENT_DIR}/bin
SOURCE_DIR=${CURRENT_DIR}/src
TEST_DIR=${CURRENT_DIR}/test

INSTALL_PREFIX=?/usr
INSTALL_INCLUDE=${INSTALL_PREFIX}/include/elf-hook
INSTALL_LIB=${INSTALL_PREFIX}/lib

${OUTPUT_DIR}:
	mkdir $@

${INTERMEDIATE_DIR}:
	mkdir $@

build_lib:
	make -C ${SOURCE_DIR} ${OUTPUT_DIR}/libelf-hook.a \
	CFLAGS=${CFLAGS} \
	INTERMEDIATE_DIR=${INTERMEDIATE_DIR} \
	OUTPUT_DIR=${OUTPUT_DIR}

build_test:
	make -C ${TEST_DIR} ${OUTPUT_DIR}/test \
	CFLAGS=${CFLAGS} \
	INTERMEDIATE_DIR=${INTERMEDIATE_DIR} \
	OUTPUT_DIR=${OUTPUT_DIR}

all: build_lib build_test

force:

rebuild: clean build

install:
	mkdir -p ${INSTALL_INCLUDE}
	mkdir -p ${INSTALL_LIB}
	cp src/elf-hook.h ${INSTALL_INCLUDE}/elf-hook.h
	cp src/externc.h ${INSTALL_INCLUDE}/externc.h
	cp src/dl-info.h ${INSTALL_INCLUDE}/dl-info.h
	cp bin/libelf-hook.a ${INSTALL_LIB}/libelf-hook.a

clean: force
	rm -rf ${INTERMEDIATE_DIR}
	rm -rf ${OUTPUT_DIR}

.PHONY: all clean test rebuild install force
