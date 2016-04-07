#!/bin/bash -x

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
    docker cp . fruit:/
    
    docker exec fruit bash -c "
        export COMPILER=$COMPILER; 
        export N_JOBS=$N_JOBS; 
        export STLARG=$STLARG; 
        scripts/postsubmit-helper.sh $1"
    ;;

osx)
    scripts/postsubmit-helper.sh "$@"
    ;;

*)
    echo "Unsupported OS: $OS"
    exit 1
esac
