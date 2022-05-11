#!/bin/bash

python3 ./scripts/cpplint.py \
    --filter=-whitespace/indent,-build/include_order,-build/include_subdir,-build/namespaces,-build/c++11 \
    --includeorder=standardcfirst \
    --exclude=src/urpc/endpoint.cc \
    --exclude=src/urpc/iobuf.cc \
    --exclude=src/urpc/iobuf.h \
    --recursive \
    src include
