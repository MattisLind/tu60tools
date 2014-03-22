#!/bin/bash
cc -mmacosx-version-min=10.6 TU60write.c hdlcproto.c -o TU60write
cc -mmacosx-version-min=10.6 TU60read.c hdlcproto.c -o TU60read
