DATA_DIR?=out/chromium-data-dir
CHROME_PATH?=/home/binji/dev/chromium/src/out/Release/chrome
CHROME_ARGS?=--user-data-dir=${DATA_DIR} --enable-nacl ${CHROME_EXTRA_ARGS} --ignore-gpu-blacklist

OUT_DIR=out
BUILD_NINJA=build.ninja
NACL_SDK_ROOT=${OUT_DIR}/nacl_sdk/pepper_33
#NACL_SDK_ROOT=/home/binji/dev/chromium/src/out/pepper_33
NINJA=${OUT_DIR}/ninja
NINJA_WRAP=build/ninja-wrap/ninja_wrap.py
RUN=${NACL_SDK_ROOT}/tools/run.py

all: ${BUILD_NINJA} ${NINJA} ${NACL_SDK_ROOT}
	@${NINJA}

love_debug: ${BUILD_NINJA} ${NINJA} ${NACL_SDK_ROOT}
	@${NINJA} ${OUT_DIR}/love_debug.nmf

love_release: ${BUILD_NINJA} ${NINJA} ${NACL_SDK_ROOT}
	@${NINJA} ${OUT_DIR}/love_release.nmf

${OUT_DIR}:
	@mkdir -p ${OUT_DIR}

${NINJA}: | ${OUT_DIR}
	@echo "installing ninja"
	@cd third_party/ninja && ./bootstrap.py
	@cp third_party/ninja/ninja ${NINJA}

${NACL_SDK_ROOT}: | ${OUT_DIR}
	@echo "installing nacl_sdk"
	@./build/install_sdk.sh

${BUILD_NINJA}: build/build.nw ${NINJA_WRAP}
	@python ${NINJA_WRAP} $< -o $@ -D nacl_sdk_root=${NACL_SDK_ROOT}

.PHONY: ports
ports: | ${NACL_SDK_ROOT}
	@./build/build_ports.sh

clean:
	@rm -rf ${OUT_DIR} ${BUILD_NINJA}

runclean: all
	@rm -rf ${DATA_DIR}
	@${CHROME_PATH} ${NEXE_ARGS}

run: all
	@${RUN} ${CHROME_PATH} -C ${PWD}/${OUT_DIR} -P drop.html -- ${CHROME_ARGS}

run-package: all
	@${CHROME_PATH} --load-extension=${PWD}/${OUT_DIR}/package ${CHROME_ARGS}

debug: all
	@${RUN} ${CHROME_PATH} -C ${PWD}/${OUT_DIR} -P drop.html -- ${CHROME_ARGS} --enable-nacl-debug

.PHONY: all clean runclean run run-package debug
