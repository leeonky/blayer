#!/bin/bash

bits=${2}
bits=${bits:=4}

decoder/decoder $1 -v 0 -b $bits |
vbuf/vbuf -s $(((1<<bits+1)/2)) |
vplayer/vplayer &

decoder/decoder $1 -a 0 -b $bits |
rsp/rsp |
aplayer/aplayer > /dev/null
