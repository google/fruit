#!/bin/bash

cd extras/dockerfiles/ || exit 1

# Setup for ARM
docker run --rm --privileged multiarch/qemu-user-static:register --reset

COMMANDS=()

for V in 18.04 19.10 20.04 20.10
do
  C="docker build --squash -t polettimarco/fruit-basesystem:ubuntu-$V -f Dockerfile.ubuntu-$V ."
  COMMANDS+=("$C || { echo; echo FAILED: '$C'; echo; exit 1; }")
done

for V in 18.04 20.04
do
  C="docker build --squash -t polettimarco/fruit-basesystem:ubuntu_arm-$V -f Dockerfile.ubuntu_arm-$V ."
  COMMANDS+=("$C || { echo; echo FAILED: '$C'; echo; exit 1; }")
done

for C in "${COMMANDS[@]}"
do
  echo "$C"
done | xargs -P 0 -L 1 -d '\n' bash -c || {

  # The successful ones should all be no-ops at this point, the failing ones won't be.
  # This way we get better diagnostics.
  for C in "${COMMANDS[@]}"
  do
    bash -c "$C" || exit 1
  done
}

docker push polettimarco/fruit-basesystem
