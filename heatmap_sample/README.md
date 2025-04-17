
/usr/share/bcc/tools/biosnoop > out.biosnoop

[CentOS-root@xdlinux ➜ heatmap_sample git:(main) ✗ ]$ awk 'NR > 1 { print $1, 1000 * $NF }' out.biosnoop | \
    /home/local/HeatMap/trace2heatmap.pl --unitstime=s --unitslabel=us --maxlat=2000 > out.biosnoop.svg
