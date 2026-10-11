#!/bin/bash
# Export project variables
export PRJ=gemini-wilgat
export SRC=src
export SRC_C=$SRC/main/c
export SRC_CPP=$SRC/main/cpp
export BUILD=target
export BUILD_DEBUG=$BUILD/exe/debug
# Variable name LIB reserved for ./configure
export REQUIRED_LIBS="gtk+-3.0 webkit2gtk-4.1 sqlite3 lua5.3"

# Array of prerequisite packages
REQUIRED_PACKAGES="build-essential pkg-config libgtk-3-dev libwebkit2gtk-4.1-dev libsqlite3-dev libglib2.0-dev liblua5.3-dev lua5.3 libepoxy-dev libegl1-mesa-dev git"

# Array of avoided packages
AVOIDED_PACKAGES=""

apt_install() {
  RESULT=$(apt list --installed 2>&1 | grep "$2")
  if [ -z "$RESULT" ]; then 
    if [ -z "$1" ]; then
      sudo apt update
    fi
    sudo apt install -y -qq "$2"
    echo "1"
  else 
    echo "$1"
  fi 
}

apt_uninstall() {
  if apt list --installed 2>&1 | grep -q "$1"; then 
    sudo apt purge --autoremove -y -qq "$1"
  fi 
}

install() {
  UPD=0
  echo "$REQUIRED_PACKAGES" | tr ' ' '\n' | while read -r package; do
    if [ ! -z "$package" ]; then
      UPD=$(apt_install $UPD "$package")
    fi
  done

  echo "$AVOIDED_PACKAGES" | tr ' ' '\n' | while read -r package; do
    if [ ! -z "$package" ]; then
      apt_uninstall "$package"
    fi
  done
}

# Installing packages is a script action. Sourcing this file for PRJ and
# paths must not run apt.
if [ "${BASH_SOURCE[0]}" = "$0" ]; then
  install
fi
