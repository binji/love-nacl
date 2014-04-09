set -o errexit

readonly SCRIPT_DIR=$(dirname $(readlink -f $0))
readonly ROOT_DIR=$(dirname ${SCRIPT_DIR})
readonly OUT_DIR=${ROOT_DIR}/out
readonly NACL_SDK_ROOT=${OUT_DIR}/nacl_sdk/pepper_33
readonly NACLPORTS_DIR=${SCRIPT_DIR}/naclports
readonly LIBS="\
  devil \
  freetype \
  jpeg8d \
  libmng \
  libmodplug \
  libogg \
  libpng \
  libvorbis \
  lua5.1 \
  mpg123 \
  openal-soft \
  physfs \
  sdl \
  tiff \
  zlib \
"

export NACL_SDK_ROOT
cd ${NACLPORTS_DIR}
make TOOLCHAIN=pnacl ${LIBS}
