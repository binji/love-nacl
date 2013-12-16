set -o errexit

readonly SCRIPT_DIR=$(dirname $(readlink -f $0))
readonly ROOT_DIR=$(dirname ${SCRIPT_DIR})
readonly OUT_DIR=${ROOT_DIR}/out
readonly NACL_SDK_ROOT=/home/binji/dev/chromium/src/out/pepper_33
readonly NACLPORTS_DIR=${SCRIPT_DIR}/naclports
readonly LIBS="vorbis ogg openal physfs sdl modplug mpg123 freetype DevIL mng jpeg png zlib tiff lua"

export NACL_SDK_ROOT
cd ${NACLPORTS_DIR}
make NACL_ARCH=pnacl ${LIBS}
