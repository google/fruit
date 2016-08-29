#!/bin/bash

F=$(mktemp)

find include/fruit/impl/ -type f | fgrep -v 'component_functors.defn.h' |  xargs cat \
| egrep "^(struct|class) [A-Za-z0-9_]* " \
| sed -r 's/struct ([A-Za-z0-9_]*).*/\1/' \
| egrep -v 'Error(Tag)?$' | fgrep -v Helper \
| sort | uniq \
>$F

for t in `cat $F`
do 
    egrep -qR "[^a-zA-Z0-9]$t[^a-zA-Z0-9]" tests/ || echo $t
done
