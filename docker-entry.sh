#!/bin/bash

export DATA_DIR=/data
export CFG=~/shaback.lua

shaback version

cd ${DATA_DIR}
test -d repo.properties || shaback init --repo-format=3

