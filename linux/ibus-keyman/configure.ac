# vim:set et ts=4:
#
# ibus-keyman - The Keyman engine for IBus
#
# Copyright (c) 2009-2018 SIL International
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

# if not 1, append datestamp to the version number.
m4_define([ibus_released], [0])
m4_define([ibus_major_version], [1])
m4_define([ibus_minor_version], [2])
m4_define([ibus_micro_version], [0])
m4_define(ibus_maybe_datestamp,
    m4_esyscmd([if test x]ibus_released[ != x1; then date +.%Y%m%d | tr -d '\n\r'; fi]))

m4_define([ibus_version],
    ibus_major_version.ibus_minor_version.ibus_micro_version[]ibus_maybe_datestamp)

AC_INIT([ibus-keyman], m4_esyscmd_s([cat VERSION]), [https://github.com/keymanapp/keyman/issues/new])

AM_INIT_AUTOMAKE
AC_GNU_SOURCE

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# define PACKAGE_VERSION_* variables
AM_SANITY_CHECK
AM_MAINTAINER_MODE
AM_DISABLE_STATIC
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_ISC_POSIX
AC_HEADER_STDC
AM_PROG_LIBTOOL

# check ibus
PKG_CHECK_MODULES(IBUS, [
    ibus-1.0 >= 1.2.0
])

# check Json glib
PKG_CHECK_MODULES(JSON_GLIB, [
    json-glib-1.0 >= 1.4.0
])

# check Keyman keyboard processor

PKG_CHECK_MODULES(KEYMAN_PROC, [
    keyman_kmn_processor >= 0.0.0
])


# check env
AC_PATH_PROG(ENV, env)
AC_SUBST(ENV)

# define GETTEXT_* variables
GETTEXT_PACKAGE=ibus-keyman
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Define to the read-only architecture-independent data directory.])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.19.7)


# OUTPUT files
AC_CONFIG_FILES([ po/Makefile.in
Makefile
src/Makefile
src/keyman.xml.in
icons/Makefile
m4/Makefile
])

AC_OUTPUT
