unset key
set ticslevel 0.1
set xlabel "Step"
set ylabel "Tick"
set zlabel "Count"
set zlabel rotate
set xtic .2
set ytic 1
set grid x y z
set title "Ticks Frequency"
splot("run.bins")

