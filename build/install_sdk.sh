set -o errexit

readonly SCRIPT_DIR=$(dirname $(readlink -f $0))
readonly ROOT_DIR=$(dirname ${SCRIPT_DIR})
readonly OUT_DIR=${ROOT_DIR}/out
readonly NACL_SDK_URL=http://storage.googleapis.com/nativeclient-mirror/nacl/nacl_sdk/nacl_sdk.zip

cd ${OUT_DIR}
wget ${NACL_SDK_URL}
unzip nacl_sdk.zip
nacl_sdk/naclsdk update pepper_29 --force
