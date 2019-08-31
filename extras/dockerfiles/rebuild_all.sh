#!/bin/bash

cd extras/dockerfiles/ || exit 1

# Setup for ARM
docker run --rm --privileged multiarch/qemu-user-static:register --reset

COMMANDS=()

for V in 16.04 17.04 18.04 18.10 19.04 16.04
do
  C="docker build -t polettimarco/fruit-basesystem:ubuntu-$V -f Dockerfile.ubuntu-$V ."
  COMMANDS+=("$C || { echo; echo FAILED: '$C'; echo; exit 1; }")
done

for V in 16.04 18.04
do
  C="docker build -t polettimarco/fruit-basesystem:ubuntu_arm-$V -f Dockerfile.ubuntu_arm-$V ."
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
    $C || {
      echo "Failed: $C"
      exit 1
    }
  done
}

docker push polettimarco/fruit-basesystem
