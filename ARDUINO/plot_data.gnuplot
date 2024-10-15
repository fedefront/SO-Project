
set terminal pngcairo size 800,600 enhanced font 'Verdana,10'


set output 'grafico_canali.png'


set title 'Grafico dei due canali acquisiti'
set xlabel 'Numero di campione'
set ylabel 'Valore ADC'

set grid

set xrange [0:10000]
set yrange [0:1024]
plot 'voltage.txt' using 0:1 with lines title 'Canale 1', \
     'voltage.txt' using 0:2 with lines title 'Canale 2'