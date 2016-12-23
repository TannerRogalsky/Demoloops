#!/bin/sh

palette="/tmp/palette.png"

filters="fps=25,scale=480:-1:flags=lanczos"

ffmpeg -v warning -r 50 -i frames/frame%04d.bmp -vf "$filters,palettegen" -y $palette
ffmpeg -v warning -r 50 -i frames/frame%04d.bmp -i $palette -lavfi "$filters [x]; [x][1:v] paletteuse" -y $1
