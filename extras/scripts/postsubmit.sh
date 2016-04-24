#!/bin/bash

set -e

# This marker instructs Travis CI to fold the stdout/stderr of the following commands
echo "travis_fold:start:$1"
echo "Running: export OS=$OS; export UBUNTU=$UBUNTU; export N_JOBS=$N_JOBS; export COMPILER=$COMPILER; export STL=$STL; $0 $1"

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
    docker cp . fruit:/
    
    docker exec fruit bash -c "
        export COMPILER=$COMPILER; 
        export N_JOBS=$N_JOBS; 
        export STLARG=$STLARG; 
        extras/scripts/postsubmit-helper.sh $1"
    N=$?
    ;;

osx)
    export COMPILER
    export N_JOBS
    export STLARG
    extras/scripts/postsubmit-helper.sh "$@"
    N=$?
    ;;

*)
    echo "Unsupported OS: $OS"
    exit 1
esac

echo "travis_fold:end:$1"
exit $N
