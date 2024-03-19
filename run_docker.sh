SCRIPT_PATH=$(dirname $(realpath -s $0))

DOCKER_IMG=fodinabor/llvm_workshop:dsl

docker pull ${DOCKER_IMG}

docker run \
    --rm -i -t \
    --net=host \
    -v ${SCRIPT_PATH}:${SCRIPT_PATH} \
    -w ${SCRIPT_PATH} \
    -e USER_UID=`id -u` \
    -e USER_GID=`id -g` \
    -e USER_NAME=${USER} \
    ${DOCKER_IMG}
