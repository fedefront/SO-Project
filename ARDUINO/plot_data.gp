set terminal png size 1800,1000
set output "oscilloscopio.png"

set title "Risultati Oscilloscopio" 
set xlabel "Tempo (ms)"
set ylabel "V (V)"
set autoscale
set grid
set xrange [0:10000]
set yrange [0:1024]
plot 'voltage.txt'

