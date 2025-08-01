#!/usr/bin/env bash
# Keyman is copyright (C) SIL Global. MIT License.

# shellcheck disable=SC2154
. "${KEYMAN_ROOT}/resources/build/jq.inc.sh"

# Allows for a quick macOS check for those scripts requiring a macOS environment.
verify_on_mac() {
  if ! builder_is_macos; then
    builder_die "This build script will only run in a Mac environment."
    exit 1
  fi
}

# The list of valid platforms that our build scripts ought expect.
platforms=("android" "ios" "linux" "lmlayer" "mac" "web" "win")

# Used to validate a specified 'platform' parameter.
_verify_platform() {
  match=false
  for proj in "${platforms[@]}"
  do
    if [[ "${proj}" = "$1" ]]; then
      match=true
    fi
  done

  if [[ ${match} = false ]]; then
    builder_die "Invalid project specified!"
  fi
}

displayInfo() {
  # TODO: can we replace this function with builder_debug?
  if [[ "${QUIET}" != true ]]; then
    while [[ $# -gt 0 ]] ; do
      echo "$1"
      shift # past argument
    done
  fi
}

assertFileExists() {
  if ! [[ -f $1 ]]; then
    builder_die "Build failed:  missing $1"
  fi
}

assertDirExists() {
  if ! [[ -d $1 ]]; then
    builder_die "Build failed:  missing $1"
  fi
}

assertValidVersionNbr()
{
  # REVIEW: this function doesn't seem to be used anywhere in the codebase.
  if [[ "$1" == "" || ! "$1" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    builder_die "Specified version not valid: '$1'. Version should be in the form Major.Minor.BuildCounter"
  fi
}

assertValidPRVersionNbr()
{
  # REVIEW: this function doesn't seem to be used anywhere in the codebase.
  if [[ "$1" == "" || ! "$1" =~ ^[0-9]+\.[0-9]+\.pull\.[0-9]+$ ]]; then
    builder_die "Specified version not valid: '$1'. Version should be in the form Major.Minor.pull.BuildCounter"
  fi
}

#
# Write ${UPLOAD_DIR}/${ARTIFACT_FILENAME}.download_info file for the target
# artifact
#
# Parameters:
#   1: UPLOAD_DIR          Directory where artifact can be found
#   2: ARTIFACT_FILENAME   Filename (without path) of artifact
#   3: ARTIFACT_NAME       Descriptive name of artifact
#   4: ARTIFACT_TYPE       File extension of artifact, without initial period (e.g. tar.gz)
#   5: PLATFORM            Target platform for artifact
#
write_download_info() {
  local UPLOAD_DIR="$1"
  local ARTIFACT_FILENAME="$2"
  local ARTFIACT_NAME="$3"
  local ARTIFACT_TYPE="$4"
  local PLATFORM="$5"

  local DATE HASH SIZE DOWNLOAD_INFO STAT_FLAGS

  _verify_platform "${PLATFORM}"

  # shellcheck disable=SC2312
  if builder_is_macos && [[ $(command -v stat) == /usr/bin/stat ]]; then
    # /usr/bin/stat on mac is BSD
    STAT_FLAGS="-f%z"
  else
    # GNU (coreutils)
    STAT_FLAGS="--print=%s"
  fi

  # Construct .download_info
  DATE=$(date +%F)
  # shellcheck disable=SC2312
  HASH=$(md5sum "${UPLOAD_DIR}/${ARTIFACT_FILENAME}" | cut -d ' ' -f 1)
  # shellcheck disable=SC2248
  SIZE=$(stat ${STAT_FLAGS} "${UPLOAD_DIR}/${ARTIFACT_FILENAME}")

  # shellcheck disable=SC2016,SC2154
  DOWNLOAD_INFO=$(
    "${JQ}" -n \
    --arg NAME "${ARTFIACT_NAME}" \
    --arg BUILD_NUMBER "${KEYMAN_VERSION}" \
    --arg DATE "${DATE}" \
    --arg PLATFORM "${PLATFORM}" \
    --arg KEYMAN_TIER "${KEYMAN_TIER}" \
    --arg FILENAME "${ARTIFACT_FILENAME}" \
    --arg ARTIFACT_TYPE "${ARTIFACT_TYPE}" \
    --arg HASH "${HASH}" \
    --arg BUILD_COUNTER "${KEYMAN_VERSION_PATCH}" \
    --arg SIZE "${SIZE}" \
    '{
      name: $NAME, version: $BUILD_NUMBER, date: $DATE, platform: $PLATFORM,
      stability: $KEYMAN_TIER, file: $FILENAME, md5: $HASH, type: $ARTIFACT_TYPE,
      build: $BUILD_COUNTER, size: $SIZE
    }'
  )
  echo "${DOWNLOAD_INFO}" | "${JQ}" . >> "${UPLOAD_DIR}/${ARTIFACT_FILENAME}.download_info"
}

# set_version sets the file version on mac/ios projects
set_version ( ) {
  # REVIEW: this function doesn't seem to be used anywhere in the codebase.
  PRODUCT_PATH=$1

  if [ $KEYMAN_VERSION ]; then
    if [ $2 ]; then  # $2 = product name.
      echo "Setting version numbers in $2 to $KEYMAN_VERSION."
    fi
    /usr/libexec/Plistbuddy -c "Set CFBundleVersion $KEYMAN_VERSION" "$PRODUCT_PATH/Info.plist"
    /usr/libexec/Plistbuddy -c "Set CFBundleShortVersionString $KEYMAN_VERSION" "$PRODUCT_PATH/Info.plist"
  fi
}


# Uses npm to set the current package version (package.json).
#
# This sets the version according to the current KEYMAN_VERSION_WITH_TAG.
#
# Usage:
#
#   set_npm_version
#
set_npm_version () {
  # REVIEW: this function doesn't seem to be used anywhere in the codebase.

  # We use --no-git-tag-version because our CI system controls version numbering and
  # already tags releases. We also want to have the version of this match the
  # release of Keyman Developer -- these two versions should be in sync. Because this
  # is a large repo with multiple projects and build systems, it's better for us that
  # individual build systems don't take too much ownership of git tagging. :)
  npm version --allow-same-version --no-git-tag-version --no-commit-hooks "$KEYMAN_VERSION_WITH_TAG"
}

#
# Re-runs the specified command-line instruction up to 5 times should it fail, waiting a
# random delay between each attempt.  No re-runs are attempted after successful commands.
#
# ### Usage
#   try_multiple_times command [param1 param2...]
#
# ### Parameters
#   1: $@         command-line arguments
try_multiple_times ( ) {
  _try_multiple_times 0 "$@"
}

# $1  The current retry count
# $2+ (everything else) the command to retry should it fail
_try_multiple_times ( ) {
  local RETRY_MAX=5

  # Configuration:  wait between 10 sec and 120 sec.

  # in seconds.
  local RETRY_MAX_WAIT_RANGE=111
  # in seconds
  local RETRY_MIN_WAIT=10

  local retryCount=$1
  shift

  if (( "$retryCount" == "$RETRY_MAX" )); then
    builder_die "Retry limit of $RETRY_MAX attempts reached."
  fi

  retryCount=$(( $retryCount + 1 ))

  if (( $retryCount != 1 )); then
    local wait_length=$(( RANDOM % RETRY_MAX_WAIT_RANGE + RETRY_MIN_WAIT ))
    builder_echo "Delaying $wait_length seconds before attempt $retryCount: \`$@\`"
    sleep $wait_length
  fi

  local code=0
  "$@" || code=$?
  if (( $code != 0 )); then
    builder_echo "Command failed with error $code"
    _try_multiple_times $retryCount "$@"
  fi
}

#
# Verifies that node is installed, and installs npm packages, but only once per
# build invocation
#
verify_npm_setup() {
  # We'll piggy-back on the builder module dependency build state to determine
  # if npm ci has been called in the current script invocation. Adding the
  # prefix /external/ to module name in order to differentiate between this and
  # internal modules (although it is unlikely to ever collide!); we will also
  # use this pattern for other similar external dependencies in future. These
  # functions are safe to call even in a non-builder context (they do nothing or
  # return 1 -- not built)
  if builder_has_module_been_built /external/npm-ci; then
    return 0
  fi
  builder_set_module_has_been_built /external/npm-ci

  # If we are on CI environment, automatically select a node version with nvm
  # Also, a developer can set KEYMAN_USE_NVM variable to get this behaviour
  # automatically too (see /docs/build/node.md)
  if [[ "$KEYMAN_VERSION_ENVIRONMENT" != local || ! -z "${KEYMAN_USE_NVM+x}" ]]; then
    _select_node_version_with_nvm
  fi

  # Check if Node.JS/npm is installed.
  type npm >/dev/null ||\
    builder_die "Build environment setup error detected!  Please ensure Node.js is installed!"

  pushd "$KEYMAN_ROOT" > /dev/null

  offline_param=
  if builder_try_offline; then
    builder_echo "Trying offline build"
    offline_param=--prefer-offline
  fi
  try_multiple_times npm ${offline_param} ci

  popd > /dev/null
}

_print_expected_node_version() {
  "$JQ" -r '.engines.node' "$KEYMAN_ROOT/package.json"
}

# Use nvm to select a node version according to package.json
# see /docs/build/node.md
_select_node_version_with_nvm() {
  local REQUIRED_NODE_VERSION  CURRENT_NODE_VERSION

  REQUIRED_NODE_VERSION="$(_print_expected_node_version)"

  if builder_is_windows; then
    CURRENT_NODE_VERSION="$(node --version)"
    if [[ "${CURRENT_NODE_VERSION}" != "v${REQUIRED_NODE_VERSION}" ]]; then
      start //wait //b nvm install "${REQUIRED_NODE_VERSION}"
      start //wait //b nvm use "${REQUIRED_NODE_VERSION}"
    fi
  else
    # launch nvm in a sub process, see _builder_nvm.sh for details
    "${KEYMAN_ROOT}/resources/build/_builder_nvm.sh" "${REQUIRED_NODE_VERSION}"
  fi

  # Now, check that the node version is correct, on all systems

  # Note: On windows, `nvm use` and `nvm install` always return success.
  # https://github.com/coreybutler/nvm-windows/issues/738

  # note the 'v' prefix that node emits (and npm doesn't!)
  CURRENT_NODE_VERSION="$(node --version)"
  if [[ "$CURRENT_NODE_VERSION" != "v$REQUIRED_NODE_VERSION" ]]; then
    builder_die "Attempted to select node.js version $REQUIRED_NODE_VERSION but found $CURRENT_NODE_VERSION instead"
  fi
}

check-markdown() {
  node "$KEYMAN_ROOT/resources/tools/check-markdown" --root "$1"
}

#
# Runs eslint, builds tests, and then runs tests with mocha + c8 (coverage)
#
# Usage:
#   builder_run_action  test    builder_do_typescript_tests [coverage_threshold]
# Parameters:
#   1: coverage_threshold   optional, minimum coverage for c8 to pass tests,
#                           defaults to 90 (percent)
#
# Todo:
#   Move to builder.typescript.inc.sh when this is established
#
builder_do_typescript_tests() {
  local MOCHA_FLAGS=

  if builder_is_running_on_teamcity; then
    # we're running in TeamCity
    MOCHA_FLAGS="-reporter mocha-teamcity-reporter"
  fi

  eslint .
  tsc --build test/

  local THRESHOLD_PARAMS=
  local C8_THRESHOLD=
  if [[ $# -gt 0 ]]; then
    C8_THRESHOLD=$1
    THRESHOLD_PARAMS="--lines $C8_THRESHOLD --statements $C8_THRESHOLD --branches $C8_THRESHOLD --functions $C8_THRESHOLD"
  else
    # Seems like a bug in TeamCity reporter if we don't list default thresholds,
    # see #13418.
    #
    # Making branch and function thresholds slightly lower, because the default
    # for c8 is 0 for these anyway.
    THRESHOLD_PARAMS="--lines 90 --statements 90 --branches 80 --functions 80"
  fi

  c8 --reporter=lcov --reporter=text --exclude-after-remap --check-coverage $THRESHOLD_PARAMS mocha ${MOCHA_FLAGS} "${builder_extra_params[@]}"

  if [[ ! -z "$C8_THRESHOLD" ]]; then
    builder_echo warning "Coverage thresholds are currently $C8_THRESHOLD%, which is lower than ideal."
    builder_echo warning "Please increase threshold in build.sh as test coverage improves."
  fi
}
