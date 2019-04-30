set term pdf
set key top left
set key font ",20"
set xtics font ",20" 
set ytics font ",20" 
set xlabel font ",20" 
set ylabel font ",20"
set bmargin 5
set lmargin 12
set ylabel "Y Label" offset -3,0,0
set xrange [-40:40]

# Key means label...
set key inside top right
set xlabel 'Number of changes in the number of students'
set ylabel 'Probability density function'
plot  "diffcap.data" using 1:2 title 'Normal' with lines, "diffcap.data" using 3:4 title 'Data' with linespoints
