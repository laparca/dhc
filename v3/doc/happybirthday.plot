set terminal pdf
set output 'happybirthday.pdf'

p(x) = (x==0)? 1 : p(int(x)-1.0)*(366.0-int(x))/365.0
pi(x) = 1 - p(x)

set xrange [0:100]
set xlabel "Número de individuos"
set ylabel "Probabilidad de colisión"
unset key
set arrow 1 from 23, pi(23) to 23, 0
set arrow 2 from 23, pi(23) to  0, pi(23)
set label "23 individuos" at graph 23, graph pi(23)
plot pi(x) title "Probabilidad de colisión"