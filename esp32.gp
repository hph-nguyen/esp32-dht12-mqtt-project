set datafile separator ","
set xdata time
set timefmt "%Y-%m-%d %H:%M:%S"
set yrange [ 10:55 ]
set format x "%m-%d \n %H:%M"
set timestamp "Last updated: %d.%m.%y %H:%M"
plot "esp32.csv" u 4:2 t "temperature" w lp ls  7 lw 2,\
"esp32.csv" u 4:3 t "humidity" w lp ls 6 lw 2
pause 60
reread
