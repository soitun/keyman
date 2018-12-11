#!/bin/bash
#----------------------------------------------------------------------------
# Detect list of files changed by a PR and trigger the appropriate CI builds
# param1 - branch name
#----------------------------------------------------------------------------

#
# This script is uses commands available to Git Bash on Windows. (mingw32)

function display_usage {
  echo "Usage: $0 [branch]"
  exit 1
}

function setup {
  if [[ "$1" = "" ]]; then
    display_usage
  fi

  echo "Checking triggers for branch: $1"
  git fetch
  git checkout master
  git pull
  git checkout $1
  git pull
}

function get_diffs {
  DO_ANDROID=false
  DO_IOS=false
  DO_KEYBOARDPROCESSOR=false
  DO_LINUX=false
  DO_MAC=false
  DO_WEB=false
  DO_WINDOWS=false
  
  mapfile -t < <( git diff --numstat master... )
  # echo "mapfile: ${MAPFILE[@]}"

  shopt -s nocasematch
  for row in ${MAPFILE[@]}; do
    # echo "row: $row"
    for column in $row; do
      # search on pathnames (ignore line numbers and history.md files)
      if [[ $column =~ ^[0-9]+$ ]] || [[ $column =~ history.md ]]; then
        continue;
      fi

      path="$column"
      echo "Change in $path"

      case "$path" in
      android*)
          DO_ANDROID=true;;
      ios*)
          DO_IOS=true;;
      common/engine/keyboardprocessor*)
          DO_KEYBOARDPROCESSOR=true;;
      linux*)
          DO_LINUX=true;;
      mac*)
          DO_MAC=true;;
      web*)
          DO_WEB=true;;
      windows*)
          DO_WINDOWS=true;;
      esac
    done
  done

  echo "DO_ANDROID: $DO_ANDROID"
  echo "DO_IOS: $DO_IOS"
  echo "DO_KPAPI: $DO_KEYBOARDPROCESSOR"
  echo "DO_LINUX: $DO_LINUX"
  echo "DO_MAC: $DO_MAC"
  echo "DO_WEB: $DO_WEB"
  echo "DO_WINDOWS: $DO_WINDOWS"


}

setup "$@"
get_diffs "$@"
