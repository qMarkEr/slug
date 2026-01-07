#!/usr/bin/bash
basedir=$(cd $(dirname $0); pwd)
sudo ln -s ${basedir}/hotslugclient.service /etc/systemd/system/hotslugclient.service
