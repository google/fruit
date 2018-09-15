#!/bin/bash

FRUIT_VERSION=3.4.0

# To authenticate:
# conan user -p <BINTRAY_API_KEY_HERE> -r fruit-bintray polettimarco

for build_type in Release Debug
do
    for is_shared in True False
    do
        for use_boost in True False
        do
            conan create . google/stable -o fruit:shared=$is_shared -o fruit:use_boost=$use_boost -s build_type=$build_type
        done
    done
done

conan remote update fruit-bintray https://api.bintray.com/conan/google/fruit
conan upload fruit/${FRUIT_VERSION}@google/stable --all -r fruit-bintray
