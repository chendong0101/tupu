#!/bin/sh
if [ `whoami` = root ]
then
    sudo -u odin sh -c '/usr/local/sbin/sogou-stop -r tupu_sql_server'
else
    sogou-stop -r tupu_sql_server
fi
