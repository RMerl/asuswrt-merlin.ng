unset key
set xlabel "Step"
set ylabel "log10(increment)"
set ylabel rotate
set xtic auto
set ytic auto
set title "Ticks Increment"
plot("run.inc")