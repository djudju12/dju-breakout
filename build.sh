#!/bin/sh

set -xe

gcc -o breakout  breakout.c `sdl2-config --libs --cflags` -std=c99 -Wall -lm  && ./breakout
