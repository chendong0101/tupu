#!/bin/sh
. /etc/profile
sh stop_searchsqlserver.sh
sh start_searchsqlserver_hb.sh
