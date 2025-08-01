#!/usr/bin/env bash
## START STANDARD BUILD SCRIPT INCLUDE
# adjust relative paths as necessary
THIS_SCRIPT="$(readlink -f "${BASH_SOURCE[0]}")"
. "${THIS_SCRIPT%/*}/../../resources/build/builder.inc.sh"
## END STANDARD BUILD SCRIPT INCLUDE

#
# TODO: when we have linux-specific tests, add them here
#       as child modules
#

builder_describe "Keyman common Linux modules" \
  clean \
  configure \
  build \
  test

builder_parse "$@"

if ! builder_is_linux; then
  builder_echo grey "Platform is not linux; skipping common/linux"
  exit 0
fi

#-------------------------------------------------------------------------------------------------------------------

builder_run_child_actions clean configure build test
