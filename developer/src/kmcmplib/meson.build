#
# Keyman is copyright (C) SIL International. MIT License.
#
# Cross platform build script to compile kmcmplib and tests
#

project('kmcmplib', 'cpp', 'c',
        version: files('../../../VERSION.md'),
        license: 'MIT',
        default_options : ['buildtype=release',
                           'cpp_std=c++17',
                           'b_vscrt=static_from_buildtype',
                           'default_library=static',
                           'debug=true'])

# Import our standard compiler defines; this is copied from
# /resources/build/standard.meson.build by build.sh, because
# meson doesn't allow us to reference a file outside its root
subdir('resources')

# ------------------------------------------------------------

subdir('include')
subdir('src')
subdir('tests')
