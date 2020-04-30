import matplotlib.pyplot as plt
import pandas as pd
import os

# if len(os.sys.argv) != 2:
#     print("Usage: plot_rtt.py <alpha>")
# alpha = os.sys.argv[1]

def plot_rtt(df):
    plt.figure(figsize=(16,3.5))
    plt.plot(df.iloc[:, 0], df.iloc[:, 1])
    plt.plot(df.iloc[:, 0], df.iloc[:, 2])
    plt.title(f"alpha = {alpha}")
    plt.xlabel("Time")
    plt.ylabel("RTT")

    num_file = len(os.listdir("images"))
    plt.savefig(os.path.join("images", f"{num_file}.png"))

def plot_packet_num(df):
    plt.figure(figsize=(16,5))
    plt.plot(df.iloc[:,0], df.iloc[:,1])
    plt.title(f"Number of packets to alpha (Total messages = 1000)")
    plt.xlabel("Alpha")
    plt.ylabel("Number of packets")

    plt.savefig(os.path.join("images", f"packet_alpha.png"))
    
if __name__ == "__main__":
    # df = pd.read_csv("rtt.csv")
    # print(df)
    # plot_rtt(df)
    df = pd.read_csv("pktnum.csv")
    print(df)
    plot_packet_num(df)