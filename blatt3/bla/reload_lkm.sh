#!/bin/bash
rmmod brpa3_959042_959218
echo 'rm done'
insmod brpa3_959042_959218.ko buffer_size=2048
echo 'load done'