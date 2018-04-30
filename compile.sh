#!/bin/bash

ftn -shared -fPIC -c mini.c
ftn -fPIC -shared -o libmini.so mini.o
