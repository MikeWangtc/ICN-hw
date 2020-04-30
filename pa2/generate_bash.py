filename="pkt.sh"

with open(filename, 'w') as f:
    f.write('gcc includetime.c -o b06901061 -std=gnu11\n')
    for i in range(100, 0, -1):
        f.write(f"./b06901061 1000 0.2 0.2 10 0 {i/100}\n")
    f.write('python3 plot.py\n')