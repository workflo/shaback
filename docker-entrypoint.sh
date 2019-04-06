#!/bin/bash -e

DATA_DIR=/backup
CFG=/root/.shaback.lua

shaback version

SHABACK_REPO_FORMAT=${SHABACK_REPO_FORMAT:-3}

init_opts=""

echo "setRepository('${DATA_DIR}')" > ${CFG}
echo "setOneFileSystem(false)" >> ${CFG}
echo "addDir('/src')" >> ${CFG}
echo "setBackupName('${HOSTNAME}')" >> ${CFG}
if [ -n "${SHABACK_PASSWORD}" ]; then
    echo "setCryptoPassword('${SHABACK_PASSWORD}')" >> ${CFG}
    init_opts="${init_opts} -p ${SHABACK_PASSWORD} -E Blowfish"
fi

echo ${SHABACK_LUA} >> ${CFG}

if [ -n "${AWS_ACCESS_KEY_ID}" ]; then
    aws configure set aws_access_key_id ${AWS_ACCESS_KEY_ID}
    aws configure set aws_secret_access_key ${AWS_SECRET_ACCESS_KEY}
    if [ -n "${AWS_DEFAULT_REGION}" ]; then
        aws configure set aws_default_region ${AWS_DEFAULT_REGION}
    fi
fi

cd ${DATA_DIR}
umask 0022

test -f repo.properties || \
    shaback init \
        --repo-format=${SHABACK_REPO_FORMAT} \
        ${init_opts} \
        ${SHABACK_INIT_OPTIONS}

if [ -n "${SHABACK_CRONTAB}" ]; then
    rm -f ${DATA_DIR}/locks/*
    echo "Starting cron"
    crontab <<< $SHABACK_CRONTAB
    exec cron -f
else
    echo "Running shaback $@"
    exec shaback ${SHABACK_OPTIONS} $@
fi
