CURRENT_DIR=$(shell pwd)
INTERMEDIATE_DIR=${CURRENT_DIR}/stage
OUTPUT_DIR=${CURRENT_DIR}/bin
SOURCE_DIR=${CURRENT_DIR}/src
TEST_DIR=${CURRENT_DIR}/test

${OUTPUT_DIR}:
	mkdir $@

${INTERMEDIATE_DIR}:
	mkdir $@

${OUTPUT_DIR}/libelf-hook.so: force
	make -C ${SOURCE_DIR} $@ \
	INTERMEDIATE_DIR=${INTERMEDIATE_DIR} \
	OUTPUT_DIR=${OUTPUT_DIR}

${OUTPUT_DIR}/test: force
	make -C ${TEST_DIR} $@ \
	INTERMEDIATE_DIR=${INTERMEDIATE_DIR} \
	OUTPUT_DIR=${OUTPUT_DIR}

all: ${OUTPUT_DIR}/libelf-hook.so ${OUTPUT_DIR}/test

test: all
force:

rebuild: clean build

clean: force
	rm -rf ${INTERMEDIATE_DIR}
	rm -rf ${OUTPUT_DIR}

.PHONY: all clean test rebuild force