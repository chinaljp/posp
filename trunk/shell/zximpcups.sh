#!/bin/bash
source ${HOME}/.bash_profile

impdate=`date -d"-1 day" +%Y%m%d`
zximpcups  -s $impdate

