#!/bin/bash
#-fbranch-probabilities -fbranch-probabilities 


#-fprofile-generate 


OPTIMIZATIONST="-fprofile-use -march=corei7-avx -mtune=corei7-avx -funroll-loops -fomit-frame-pointer -ffast-math -fstrict-aliasing -pipe -malign-double -fno-exceptions -funsafe-math-optimizations -Os -O3 -s"

OPTIMIZATIONS="-s -O3 -fexpensive-optimizations -march=native -mtune=native "

gcc main.c $OPTIMIZATIONST -o MatrixTypeConversion
strip MatrixTypeConversion

clang -O3 main.c -o MatrixTypeConversionClang

exit 0
