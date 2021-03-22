#!/bin/bash

. /etc/profile
if [ `whoami` = root ]
then
    sudo -u odin sh -c "/usr/local/sbin/sogou-start -p ./_bin/tupu_sql_server -t '\
        --flagfile=conf/tupu_sql_server.cfg' \
        -bl"
else
    /usr/local/sbin/sogou-start -p ./_bin/tupu_sql_server -t '\
        --flagfile=conf/tupu_sql_server.cfg' \
        -bl
fi
