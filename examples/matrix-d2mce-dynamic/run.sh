for i in 1 2 3 4;
#do ssh -f cluster$i /cluster/3/home/ffbli/D2MCE/examples/benchmark/matrix-d2mce-dynamic/main -s 512 -n 4;
do ssh -f cluster$i ~/D2MCE/examples/benchmark/matrix-d2mce-dynamic/main -s 512 -n 4;
done
