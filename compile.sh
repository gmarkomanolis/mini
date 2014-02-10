#!/bin/bash

gcc -Wall -fPIC -c mini.c
gcc -shared -Wl,-soname -o libmini.so mini.o
