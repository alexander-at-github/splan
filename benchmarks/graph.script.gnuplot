set terminal pdfcairo dashed # solid # postfile
set output  "graph_plot.pdf"
#set title "Energy vs. Time for Sample Data"
set xlabel "value of parameter k"
set ylabel "runtime [sec]"

#set xrange [0:28]

set size 1,0.94
set key at 21, 675

set style line 1 lt rgb "green" lw 3 pt 6
set style line 2 lt rgb "orange" lw 3 pt 2
set style line 3 lt rgb "blue" lw 3 pt 6
set style line 4 lt rgb "cyan" lw 3 pt 6
set style line 5 lt rgb "red" lw 3 pt 6
set style line 6 lt rgb "magenta" lw 3 pt 6

#plot "gnu.1944.dat" using 1:2 smooth sbezier ls 1, \
#     "gnu.1944.dat" using 1:2 with points ls 1, \
#     "gnu.429.dat" using 1:2 smooth sbezier ls 2, \
#     "gnu.429.dat" using 1:2 with points ls 2, \
#     "gnu.k.1944.dat" using 1:2 smooth sbezier ls 3, \
#     "gnu.k.1944.dat" using 1:2 with points ls 3, \
#     "gnu.k.429.dat" using 1:2 smooth sbezier ls 4, \
#     "gnu.k.429.dat" using 1:2 with points ls 4, \
#     "gnu.vo.1944.dat" using 1:2 smooth sbezier ls 5, \
#     "gnu.vo.1944.dat" using 1:2 with points ls 5, \
#     "gnu.vo.429.dat" using 1:2 smooth sbezier ls 6, \
#     "gnu.vo.429.dat" using 1:2 with points ls 6

#plot "gnu.k.1944.dat" using 1:2 notitle smooth sbezier ls 1, \
#     "gnu.k.1944.dat" using 1:2 notitle with points ls 1, \
#     1 / 0 title "A* parameter k" with linespoints ls 1, \
#     "gnu.k.429.dat" using 1:2 notitle smooth sbezier ls 2, \
#     "gnu.k.429.dat" using 1:2 notitle with points ls 2, \
#     1 / 0 title "iterative deepening parameter k" with linespoints ls 2, \
#     "gnu.vo.1944.dat" using 1:2 notitle smooth sbezier ls 3, \
#     "gnu.vo.1944.dat" using 1:2 notitle with points ls 3, \
#     1 / 0 title "A* parameter vo" with linespoints ls 3, \
#     "gnu.vo.429.dat" using 1:2 notitle smooth sbezier ls 4, \
#     "gnu.vo.429.dat" using 1:2 notitle with points ls 4, \
#     1 / 0 title "iterative deepening parameter vo" with linespoints ls 4

plot "gnu.k.429.dat" using 1:2 notitle smooth sbezier ls 1, \
     "gnu.k.429.dat" using 1:2 notitle with points ls 1, \
     1 / 0 title "iterative deepening" with linespoints ls 1, \
     "gnu.k.1944.dat" using 1:2 notitle smooth sbezier ls 2, \
     "gnu.k.1944.dat" using 1:2 notitle with points ls 2, \
     1 / 0 title "A*" with linespoints ls 2

#pause -1 "Hit any key to continue"
