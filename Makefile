DATA_DIR?=out/chromium-data-dir
CHROME_PATH?=/home/binji/dev/chromium/src/out/Release/chrome
CHROME_ARGS?=--user-data-dir=${DATA_DIR} --enable-nacl ${CHROME_EXTRA_ARGS}
NACL_SDK_ROOT?=/home/binji/dev/chromium/src/out/pepper_28
NINJA_WRAP=build/ninja-wrap/ninja_wrap.py

all: build.ninja
	@ninja

build.ninja: build/build.nw ${NINJA_WRAP}
	@python ${NINJA_WRAP} $< -o $@ -D nacl_sdk_root=${NACL_SDK_ROOT}

clean:
	@rm -rf out build.ninja

runclean: all
	@rm -rf ${DATA_DIR}
	@${CHROME_PATH} ${NEXE_ARGS}

run: all
	@${CHROME_PATH} --load-extension=${PWD}/out ${CHROME_ARGS}

run-package: all
	@${CHROME_PATH} --load-extension=${PWD}/out/package ${CHROME_ARGS}

debug: all
	@${CHROME_PATH} --load-extension=${PWD}/out ${CHROME_ARGS} --enable-nacl-debug

.PHONY: all clean runclean run
