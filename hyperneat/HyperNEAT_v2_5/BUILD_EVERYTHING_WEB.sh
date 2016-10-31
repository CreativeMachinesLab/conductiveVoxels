#!/bin/sh -x

HyperNEAT_v2_5/BUILD_EVERYTHING_AUTO.sh

cp -v HyperNEAT_v2_5/out/Hypercube_NEAT web/exec/ || echo "ERROR: COPY FAILED!!!"
