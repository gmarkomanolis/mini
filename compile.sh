#!/bin/bash

cc -shared -fPIC -c mini.c
cc -fPIC -shared -o libmini.so mini.o
