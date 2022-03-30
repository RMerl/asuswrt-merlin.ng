#!/bin/sh
make
./easylogo linux_logo.tga u_boot_logo video_logo.h
mv video_logo.h ../../include
