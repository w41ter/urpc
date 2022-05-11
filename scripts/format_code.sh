#!/bin/bash

usage() {
    echo "Usage: $0 [all|commit]"
    exit 1
}

if [[ $# != 1 ]]; then
    usage
fi

format_all() {
    clang-format -i -style=file */*.cc */*.h
}

format_commit() {
    git clang-format --style=file --extension cc,h
}

case $1 in
"all")
    format_all
    ;;
"commit")
    format_commit
    ;;
*)
    echo "unknown command $1"
    usage
    ;;
esac
