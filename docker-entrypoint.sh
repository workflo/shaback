#!/bin/bash -e

DATA_DIR=/backup
CFG=/root/.shaback.lua

shaback version
echo

SHABACK_REPO_FORMAT=${SHABACK_REPO_FORMAT:-3}

init_opts=""

echo "setRepository('${DATA_DIR}')" > ${CFG}
echo "setOneFileSystem(false)" >> ${CFG}
echo "addDir('/src')" >> ${CFG}
echo "setBackupName('${HOSTNAME}')" >> ${CFG}
if [ -n "${SHABACK_PASSWORD}" ]; then
    echo "setCryptoPassword('${SHABACK_PASSWORD}')" >> ${CFG}
    init_opts="${init_opts} -p ${SHABACK_PASSWORD} -E AES"
fi

echo ${SHABACK_LUA} >> ${CFG}

cd ${DATA_DIR}
test -f repo.properties || \
    shaback init \
        --repo-format=${SHABACK_REPO_FORMAT} \
        ${init_opts} \
        ${SHABACK_INIT_OPTIONS}

if [ -n "${SHABACK_CRONTAB}" ]; then
    echo "Starting cron"
    crontab <<< $SHABACK_CRONTAB
    exec cron -f
else
    echo "Running shaback $@"
    exec shaback ${SHABACK_OPTIONS} $@
fi
