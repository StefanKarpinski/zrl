#!/usr/bin/env bash

for i in {1..11}; do
    cmp <(./zrle data/$i.raw) data/$i.zrl
done

for i in {1..11}; do
    cmp <(./zrld data/$i.zrl) data/$i.raw
done

cmp <(head -c1000 /dev/zero | ./zrle) <(printf '\0\xe7\x07')
cmp <(printf '\0\xe7\x07' | ./zrld) <(head -c1000 /dev/zero)

for n in {0..512}; do
    cmp <(head -c$n /dev/zero; yes | head -c$n) \
       <((head -c$n /dev/zero; yes | head -c$n) | ./zrle | ./zrld)
done
