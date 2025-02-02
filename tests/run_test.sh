#!/bin/bash

gcc attrstest.c -o attrstest -lncurses
gcc window.c -o window -lncurses
./attrstest
./window
rm attrstest
rm window
