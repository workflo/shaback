#!/bin/bash -e

DATA_DIR=/backup
CFG=/etc/shaback.lua

shaback version
echo

SHABACK_REPO_FORMAT=${SHABACK_REPO_FORMAT:-3}

env
init_opts=""

echo "setRepository('${DATA_DIR}')" > ${CFG}
echo "setOneFileSystem(true)" >> ${CFG}
if [ -n ${SHABACK_PASSWORD} ]; then
    echo "setCryptoPassword('${SHABACK_PASSWORD}')" >> ${CFG}
    init_opts="${init_opts} -p ${SHABACK_PASSWORD} -E AES"
fi

echo ${SHABACK_LUA} >> ${CFG}

cat ${CFG}

cd ${DATA_DIR}
test -d repo.properties || \
    shaback init \
        --repo-format=${SHABACK_REPO_FORMAT} \
        ${init_opts} \
        ${SHABACK_INIT_OPTIONS}

cat ${DATA_DIR}/repo.properties

shaback backup -c ${CFG} ${SHABACK_OPTIONS}
