#!/usr/bin/env bash

# zrl encoding & decoding

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

cmp <(./zrle /dev/null) /dev/null
cmp <(./zrld /dev/null) /dev/null

# leb128 encoding & decoding

for n in {0..127}; do
    x=$(printf "%x" $n)
    cmp <(printf "\\x$x" | ./leb128e) <(printf "\\x$x")
    cmp <(printf "\\x$x\0\0\0\0\0\0\0" | ./leb128e) <(printf "\\x$x")
    cmp <(printf "\\x$x" | ./leb128d) <(printf "\\x$x\0\0\0\0\0\0\0")
done

cmp <(printf '\0' | ./leb128d) <(printf '\0\0\0\0\0\0\0\0')
cmp <(printf '\x80\0' | ./leb128d) <(printf '\0\0\0\0\0\0\0\0')
cmp <(printf '\x80\x80\x80\x80\x80\x80\x80\x80\x80\0' | ./leb128d) \
    <(printf '\0\0\0\0\0\0\0\0')

cmp <(printf '\xff\xff\xff\xff\xff\xff\xff\x7f' | ./leb128e) \
    <(printf '\xff\xff\xff\xff\xff\xff\xff\xff\x7f')
cmp <(printf '\xff\xff\xff\xff\xff\xff\xff\xff\x7f' | ./leb128d) \
    <(printf '\xff\xff\xff\xff\xff\xff\xff\x7f')

cmp <(printf '\xff\xff\xff\xff\xff\xff\xff\xff' | ./leb128e) \
    <(printf '\xff\xff\xff\xff\xff\xff\xff\xff\xff\x01')
cmp <(printf '\xff\xff\xff\xff\xff\xff\xff\xff\xff\x01' | ./leb128d) \
    <(printf '\xff\xff\xff\xff\xff\xff\xff\xff')

cmp <(printf '\0\0\0\0\0\0\0\x80' | ./leb128e) \
    <(printf '\x80\x80\x80\x80\x80\x80\x80\x80\x80\x01')
cmp <(printf '\x80\x80\x80\x80\x80\x80\x80\x80\x80\x01' | ./leb128d) \
    <(printf '\0\0\0\0\0\0\0\x80')

cmp <(printf '\x09\xc4\xc1\x50\x20\x0c\x04\x01' | ./leb128e) \
    <(printf '\x89\x88\x87\x86\x85\x84\x83\x82\x01')
cmp <(printf '\x89\x88\x87\x86\x85\x84\x83\x82\x01' | ./leb128d) \
    <(printf '\x09\xc4\xc1\x50\x20\x0c\x04\x01')

# error checking

err=$(! printf '\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff' | ./leb128d 2>&1)
test err == "Too large LEB128 value in file '<stdin>'"

err=$(! printf '\x80\x80\x80\x80\x80\x80\x80\x80\x80\x02' | ./leb128d 2>&1)
test err == "Too large LEB128 value in file '<stdin>'"

err=$(! printf '\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80' | ./leb128d 2>&1)
test err == "Too large LEB128 value in file '<stdin>'"

cmp <(printf '\0\xff\xff\xff\xff\xff\xff\xff\xff\xff\x01' | ./zrld | head -c1000) \
    <(head -c1000 /dev/zero)

err=$(! printf '\0\xff\xff\xff\xff\xff\xff\xff\xff\xff\x02' | ./zrld 2>&1)
test err == "Too large LEB128 value in file '<stdin>'"

err=$(! printf '\xff' | ./leb128d 2>&1)
test "$err" == "Premature end of file '<stdin>'"

err=$(! printf '\x80' | ./leb128d 2>&1)
test "$err" == "Premature end of file '<stdin>'"

err=$(! printf '\0\xff' | ./zrld 2>&1)
test "$err" == "Premature end of file '<stdin>'"

err=$(! printf '\0\x80' | ./zrld 2>&1)
test "$err" == "Premature end of file '<stdin>'"
