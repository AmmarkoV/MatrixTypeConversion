#!/bin/bash
#-fbranch-probabilities -fbranch-probabilities 


OPTIMIZATIONST="-march=corei7 -mtune=corei7 -funroll-loops -fomit-frame-pointer -ffast-math -fstrict-aliasing -pipe -malign-double -fno-exceptions -funsafe-math-optimizations -Os -O3 -s"

OPTIMIZATIONS="-s -O3 -fexpensive-optimizations -march=native -mtune=native"

gcc main.c $OPTIMIZATIONST -o MatrixTypeConversion
strip MatrixTypeConversion

clang -O3 main.c -o MatrixTypeConversionClang

exit 0
