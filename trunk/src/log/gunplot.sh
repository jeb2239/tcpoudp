 #! /bin/bash
 gnuplot -persist <<EOF
 set data style linespoints
 show timestamp
 set title "ToU cwnd/sshresh"
 set xlabel "time (seconds)"
 set ylabel "Segments (cwnd, ssthresh)"
 plot "$1" using 1:2 title "snd_cwnd"
 EOF

# "$1" using 1:(\$3>=2147483647 ? 0 : \$3) title "snd_ssthresh"
