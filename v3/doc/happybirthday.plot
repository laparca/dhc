#set terminal pdf
#set output 'out/birthday.pdf'

p(x) = (366-x)/365
pi(x) = 1 - p(x)

set xrange [0:100]
plot 