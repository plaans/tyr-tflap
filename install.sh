#!/bin/bash

set -e
cd "${0%/*}" || exit 1;   # Go to the script location

if test ! -f tflap.sif
then
    echo "Compiling tflap."
    cd src/src
    export SINGULARITY_BUILDDEF="$(pwd)/Singularity"
    export SINGULARITY_ROOTFS="/tmp"
    apptainer build ../../tflap.sif Singularity
else
    echo "tflap already compiled."
fi
