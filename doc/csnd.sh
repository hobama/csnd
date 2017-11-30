#!/bin/sh
# CSND wrapper script for CentOS
export CSND_HOME=/opt/csnd
export LD_LIBRARY_PATH=${CSND_HOME}/lib:${LD_LIBRARY_PATH}

exec ${CSND_HOME}/bin/csnd "$@"
