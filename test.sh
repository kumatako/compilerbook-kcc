#!/bin/bash
assert() {
	expected="$1"
	input="$2"
	
	./kcc "$input" > tmp.s
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

assert 1 "1==-2+3;"
assert 1 "5+3!=5*3;"
assert 0 "2 <= -1;"
assert 1 "3+4 < 5*4;"
assert 1 "2*(2+2) > 2;"
assert 0 "1 >= 3+2;"
assert 4 "1+1; 2+2;"
assert 2 "a=1; 1+1;"
assert 3 "a=3; a;"
assert 7 "a=1; b=a+1; c=a+b+1; a+b+c;"
assert 3 "foo=1; bar=3; foo*bar;"
assert 3 "return 1+2;"
assert 1 "return 1; return 2;"
assert 1 "a=1; if(a==1) return 1; return 2;"
assert 2 "a=2; if(a==1) return 1; return 2;"

echo OK