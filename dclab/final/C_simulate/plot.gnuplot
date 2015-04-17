#!/usr/bin/env gnuplot
name="`cat`"
set terminal png
set output name."-x.png"
plot \
'./'.name.'.0.txt' using 1 with lp lw 2,\
'./'.name.'.1.txt' using 1 with lp lw 2,\
'./'.name.'.2.txt' using 1 with lp lw 2
set output name.'-y.png'
plot \
'./'.name.'.0.txt' using 2 with lp lw 2,\
'./'.name.'.1.txt' using 2 with lp lw 2,\
'./'.name.'.2.txt' using 2 with lp lw 2
set output name.'-theta.png'
plot \
'./'.name.'.0.txt' using 3 with lp lw 2,\
'./'.name.'.1.txt' using 3 with lp lw 2,\
'./'.name.'.2.txt' using 3 with lp lw 2
