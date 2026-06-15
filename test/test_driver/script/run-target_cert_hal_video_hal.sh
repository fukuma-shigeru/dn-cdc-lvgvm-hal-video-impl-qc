#!/bin/bash
umask 0

# If enabled, log output setting will be default behavior.
#export APPMNG_GTEST_DISABLE_DYNAMIC_LOG_LEVEL_CHANGE=1

#-----------------------------------------------------
# Setup environment variable
#-----------------------------------------------------
environment_files=(
    "/opt/arene/etc/common.conf"
    "/opt/arene/etc/cockpit.conf"
)
for file in "${environment_files[@]}"; do
    if [ -f ${file} ]; then
        export $(cat ${file} | sed s/#.*// | xargs)
    else
        echo "${file} not found." >&2
    fi
done
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/share/stub/video_hal_svc

readonly TIMEOUT=3
readonly SERVICE_APP_MNG="blkmng_app_management"

#-----------------------------------------------------
# Waiting for automatic test related service startup
#-----------------------------------------------------
SECONDS=0
while :
do
  # Service startup check
  am=`systemctl status $SERVICE_APP_MNG | grep -e Active.*running | wc -l`

  if [ $am -eq 1 ]
  then
    break
  fi

  # Timeout check
  time=$SECONDS
  if [ $time -ge $TIMEOUT ]
  then
    # Abort when timeout occurs.
    echo "timedout : Waiting for blkmng_app_management to start"
    exit 1
  fi

  # 0.5sec wait
  usleep 500000
done

export SCRIPT_DIR=$(cd $(dirname $0) && pwd)
export COCKPIT_RUNTIME_ENABLE_CERTIFICATION=1

#-----------------------------------------------------
# Usage
#-----------------------------------------------------
#usage()
#{
#  echo "usage:"
#  echo " ${0} -<short option> value | --<long option> value"
#  echo "option"
#  echo " -t | --type   : ut | it | st (default 'all')"
#  echo " -l | --level  : 1, 2, 3 (default '1')"
#  echo " -k | --key    : uniq test key (default 'all')"
#  echo " -o | --output : output directory (default /tmp/)"
#  echo "ex:"
#  echo " ${0} -t ut"       # ut
#  echo " ${0} -t it -l 1"  # it1
#  echo " ${0} -t it -l 2"  # it2
#  echo " ${0} -t it -l 3"  # it3
#  echo " ${0} --type st -o /var/log/arene-test-report/my-repository/"  # st
#}

#-----------------------------------------------------
# Option determination
#-----------------------------------------------------
#OPTION_TEST_TYPE="all"
#OPTION_TEST_LEVEL="1"
#OPTION_TEST_KEY=""
OPTION_TEST_OUTPUT="/tmp/"
#OPTION_TEST_EXT=""
#
#while [ $# -gt 0 ]
#do
#  case $1 in
#    # help
#    -h | --help)
#      shift
#      ;;
#    # ut, it, st, more
#    -t | --type)
#      shift
#      echo --type: $1
#      OPTION_TEST_TYPE="${1}"
#      shift
#      ;;
#    # test level
#    -l | --level)
#      shift
#      echo --level: $1
#      OPTION_TEST_LEVEL="${1}"
#      shift
#      ;;
#    # uniq test key
#    -k | --key)
#      shift
#      echo --key: $1
#      OPTION_TEST_KEY="${1}"
#      shift
#      ;;
#    -o | --output)
#      shift
#      echo --output: $1
#      OPTION_TEST_OUTPUT="${1}"
#      shift
#      ;;
#    -*)
#      shift
#      shift
#      ;;
#    *)
#      echo --ext: $1
#      OPTION_TEST_EXT="${1}"
#      shift
#      ;;
#  esac
#done

#-----------------------------------------------------
# Automatic test execution
#-----------------------------------------------------
${SCRIPT_DIR}/test_driver_video_hal --gtest_output=xml:${OPTION_TEST_OUTPUT}/test-videohal-ct.xml

exit 0
