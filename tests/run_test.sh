#!/bin/bash

compile() {
  gcc -o "$1" "$2" $3
  if [ $? -ne 0 ]; then
    echo "Error compiling $2"
    exit 1
  fi
}

run_program() {
    ./$1
}

cleanup() {
    rm -f "$1"
}

compile "attrstest" "attrstest.c" "-lncurses"
compile "window" "window.c" "-lncurses"
compile "forum" "forum.c" "-lncurses -lpanel"

run_program "forum"

cleanup "attrstest"
cleanup "window"
cleanup "forum"
