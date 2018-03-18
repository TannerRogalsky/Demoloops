#!/bin/sh
ffmpeg -r 60 -i frames/frame%04d.bmp -crf 0 -vcodec libx264 -pix_fmt yuv420p $1
