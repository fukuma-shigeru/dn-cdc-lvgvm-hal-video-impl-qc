#!/bin/bash

############################################
# Name      : Video HAL Test Program for IT
#            (Execution Script)
# File Name : run-target-test_hal_tp.sh
# Date      : 
############################################

#-----------------------------------------------------
# Option determination
#-----------------------------------------------------
OPTION_TEST_TYPE="all"
OPTION_TEST_LEVEL="1"
OPTION_TEST_OUTPUT="/tmp"
OPTION_TEST_FILTER=""
NEGATIVE_FILTER=""
POSITIVE_FILTER=""
TP_NAME="video_hal_tp"
DIR_NUMBER="000000"
BMP_ARCHIVE="bmp_files.tar.gz"

VERBOSE_FLAG=0

###############################################
# 機器名からgtest_filter用のフィルタ文字列を設定する。
# Arguments:
#   カンマ区切りの機器名
# Globals:
#   POSITIVE_FILTER
#   NEGATIVE_FILTER
###############################################
function add_filter_by_device() {
  local ORG_IFS=$IFS
  IFS=’,’
  local csv="$1"
  read -ra arr <<< "$csv"
  local target=""
  local front_flg=0
  local rear_flg=0
  local ic_flg=0
  local hdmi_flg=0
  local camera_flg=0

  for var in "${arr[@]}" ;do
    case ${var} in
      front)
        front_flg=1
        ;;
      rear)
        rear_flg=1
        ;;
      ic)
        ic_flg=1
        ;;
      hdmi)
        hdmi_flg=1
        ;;
      camera)
        camera_flg=1
        ;;
    esac
  done

  # 外部機器を使うテストはテスト名にキーワードを含めている。
  # Instrument Cluster=Ic、カメラ=Camera、後席=Rear、HDMI=Hdmi
  # 外部機器を使用しない試験=前席試験はこれらがテスト名に含まれていないテスト。
  # 以下のルールでフィルタ用文字列を作る。
  # キーワードにfrontなし=指定キーワードをポジティブフィルタに設定する。
  # キーワードにfrontあり=未指定キーワードをネガティブフィルタに指定する。
  if [ $front_flg = 0 ] ; then
    if [ $rear_flg = 1 ] ; then
      POSITIVE_FILTER+=":*Rear*"
      POSITIVE_FILTER+=":*Rse*"
    fi
    if [ $ic_flg = 1 ] ; then
      POSITIVE_FILTER+=":*Ic*"
    fi
    if [ $hdmi_flg = 1 ] ; then
      POSITIVE_FILTER+=":*Hdmi*"
    fi
    if [ $camera_flg = 1 ] ; then
      POSITIVE_FILTER+=":*Camera*"
    fi
  elif [ $front_flg = 1 ] ; then
    if [ $rear_flg = 0 ] ; then
      NEGATIVE_FILTER+=":*Rear*"
      NEGATIVE_FILTER+=":*Rse*"
    fi
    if [ $ic_flg = 0 ] ; then
      NEGATIVE_FILTER+=":*Ic*"
    fi
    if [ $hdmi_flg = 0 ] ; then
      NEGATIVE_FILTER+=":*Hdmi*"
    fi
    if [ $camera_flg = 0 ] ; then
      NEGATIVE_FILTER+=":*Camera*"
    fi
  fi

  # 環境異常評価
  NEGATIVE_FILTER+=":*EnvTest*"
  # 外部機器未接続
  NEGATIVE_FILTER+=":*NotConnected*"
  # 設定前の参照
  NEGATIVE_FILTER+=":*NotSet*"
  # Camera powered off
  NEGATIVE_FILTER+=":*CameraOff*"

  IFS=$ORG_IFS
}

###############################################
# ファイルからフィルタ文字列を設定する。
# +開始行はポジティブフィルタ、
# -開始行はネガティブフィルタに追加する。
# Arguments:
#   ファイルパス
# Globals:
#   POSITIVE_FILTER
#   NEGATIVE_FILTER
###############################################
function add_filter_by_file() {
  local filename=$1
  local target=""

  while read line
  do
    if [[ ${line:0:1} == "+" ]]; then
      POSITIVE_FILTER+=":"${line:1}
    fi

    if [[ ${line:0:1} == "-" ]]; then
      NEGATIVE_FILTER+=":"${line:1}
    fi
  done < $filename
}

###############################################
# ログ格納ディレクトリ名に設定する番号を振り出す。
# Globals:
#   DIR_NUMBER
###############################################
function assign_dir_number() {
  local max_value=0

  for var in `ls`
  do
    [[ $var =~ ^[0-9]{6}$ ]] && [ ${var} -gt ${max_value} ] && max_value=$var
  done

  DIR_NUMBER=`expr $max_value + 1`
  DIR_NUMBER=`printf %06d $DIR_NUMBER`
}

###############################################
# ログを圧縮し、格納用ディレクトリへ移動する。
###############################################
function compress_logfiles() {
  local base_dir_path=${HOME}/tmp/${TP_NAME}-log
#  local dir_name=`date "+%Y%m%d-%H%M%S%3N"`
  mkdir -p ${base_dir_path}; [ $? != 0 ] && return
  cd ${base_dir_path}; [ $? != 0 ] && return
  assign_dir_number
  local dir_name=$DIR_NUMBER
  mkdir -p ${base_dir_path}/${dir_name}; [ $? != 0 ] && return
  dlt-logstorage-ctrl -s /var/log/dlt
  cp /var/log/dlt/dten_videohal.*.dlt.gz ${dir_name}
  cp /var/log/dlt/arenehal.*.dlt.gz ${dir_name}
  mv ${LOG_FILE_NAME} ./${dir_name}
  mv ${XML_FILE_NAME} ./${dir_name}
  # 移動先容量が不足のためbmpは先に圧縮してから移動する。
  if ls ${OPTION_TEST_OUTPUT}/*.bmp 1> /dev/null 2>&1; then
    (cd ${OPTION_TEST_OUTPUT} && tar cf - *.bmp | gzip -n -9) > $BMP_ARCHIVE
  fi
  [ -f $BMP_ARCHIVE ] && mv $BMP_ARCHIVE ./${dir_name}
  mv ${OPTION_TEST_OUTPUT}/*.png ./${dir_name}
  sync;sync;
  tar cvzf ${dir_name}.tar.gz ${dir_name}
  cd -
}

for opt in "${@}"
do
  case ${opt} in
    # help
    -h | --help)
      shift
      ;;
    # ut, it, st, more
    -t | --type)
      shift
      echo --type: $1
      OPTION_TEST_TYPE="${1}"
      shift
      ;;
    # test level
    -l | --level)
      shift
      echo --level: $1
      OPTION_TEST_LEVEL="${1}"
      shift
      ;;
    -o | --output)
      shift
      echo --output: $1
      OPTION_TEST_OUTPUT="${1}"
      shift
      ;;
    # gtest filter parameter
    -f | --filter)
      shift
      echo --filter $1
      POSITIVE_FILTER=""
      NEGATIVE_FILTER=""  
      OPTION_TEST_FILTER="${1}"
      shift
      ;;
    # filter testcase by device name
    # device name:front,rear,ic,hdmi,camera
    -d | --device)
      shift
      echo --device $1
      POSITIVE_FILTER=""
      NEGATIVE_FILTER=""
      add_filter_by_device $1
      shift
      ;;
    # filter test case by file
    -i | --input)
      shift
      echo --input $1
      POSITIVE_FILTER=""
      NEGATIVE_FILTER=""  
      add_filter_by_file $1
      shift
      ;;
    # output log to terminal
    -v | --verbose)
      shift
      VERBOSE_FLAG=1
      ;;
    -*)
      shift
      shift
      ;;
  esac
done

#-----------------------------------------------------
# Automatic test execution
#-----------------------------------------------------
if [[ ${POSITIVE_FILTER} != "" ]]; then
  if [[ ${POSITIVE_FILTER:0:1} == ":" ]]; then
    POSITIVE_FILTER=${POSITIVE_FILTER:1}
  fi
  OPTION_TEST_FILTER=${POSITIVE_FILTER}
fi

if [[ ${NEGATIVE_FILTER} != "" ]]; then
  if [[ ${NEGATIVE_FILTER:0:1} == ":" ]]; then
    NEGATIVE_FILTER=${NEGATIVE_FILTER:1}
  fi
  NEGATIVE_FILTER="-"${NEGATIVE_FILTER}
  if [[ ${OPTION_TEST_FILTER} != "" ]]; then
    NEGATIVE_FILTER=":"${NEGATIVE_FILTER}
  fi
  OPTION_TEST_FILTER=${OPTION_TEST_FILTER}${NEGATIVE_FILTER}
fi

if [[ ${OPTION_TEST_FILTER} == "" ]] ; then
  OPTION_TEST_FILTER="*"
fi

XML_FILE_NAME=${OPTION_TEST_OUTPUT}/${TP_NAME}.xml
LOG_FILE_NAME=${OPTION_TEST_OUTPUT}/${TP_NAME}_output.txt

OUTPUT_DEST=" > ${LOG_FILE_NAME} 2>&1"

if [[ ${VERBOSE_FLAG} = 1 ]] ; then
  OUTPUT_DEST="2>&1 | tee ${LOG_FILE_NAME}"
fi

set -f
#eval "/opt/dc-ivi-pf/bin/${TP_NAME} --gtest_filter="${OPTION_TEST_FILTER}" --gtest_output=xml:${XML_FILE_NAME} ${OUTPUT_DEST}"
TP_EXEC="./${TP_NAME} --gtest_filter='${OPTION_TEST_FILTER}' --gtest_output=xml:${XML_FILE_NAME} ${OUTPUT_DEST}"
TP_CMD=eval
if [[ `whoami` == root ]] ; then
  TP_CMD="su default -c "
fi
cd /opt/dc-ivi-pf/tests/dn-cdc-lvgvm-hal-video-impl-qc/video_hal_test_module/
${TP_CMD} "${TP_EXEC}"
cd - >/dev/null
set +f
compress_logfiles

