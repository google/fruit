#!/bin/bash

set -e

: ${N_JOBS:=2}

if [ "$STL" != "" ]
then
  STLARG="-stdlib=$STL"
fi

case $OS in
linux)
    docker rm -f fruit &>/dev/null || true
    docker run -d -it --name fruit --privileged polettimarco/fruit-basesystem:ubuntu-$UBUNTU
    docker exec fruit mkdir fruit
    docker cp . fruit:/fruit
    
    docker exec fruit bash -c "
        export COMPILER=$COMPILER; 
        export N_JOBS=$N_JOBS;
        export STLARG=$STLARG; 
        export ASAN_OPTIONS=$ASAN_OPTIONS;
        export OS=$OS;
        cd fruit; extras/scripts/postsubmit-helper.sh $1"
    exit $?
    ;;

osx)
    export COMPILER
    export N_JOBS
    export STLARG
    export ASAN_OPTIONS
    export OS
    extras/scripts/postsubmit-helper.sh "$@"
    exit $?
    ;;

*)
    echo "Unsupported OS: $OS"
    exit 1
esac
