#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./f-cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 0
assert 42 42
assert 21 "5+20-4"
assert 41 " 12 + 34 - 5"
assert 47 '5+6*7'
assert 15 '5*(9-6)'
assert 4 '(3+5)/2'
assert 2 '-(2-4)'
assert 10 '+5+2+3'
assert 0 '0==1'
assert 1 '1==1'
assert 1 '1!=0'
assert 0 '2!=2'
assert 0 '2<0'
assert 1 '0<2'
assert 0 '1<1'
assert 1 '1<=1'
assert 0 '2<=1'
assert 1 '1>0'
assert 0 '1>1'
assert 0 '1>2'
assert 1 '2>=0'
assert 1 '2>=2'
assert 0 '0>=1'
assert 1 'a = 1'
echo OK
