import socket
import re
import threading

# Specify the IP addr and port number 
# (use "127.0.0.1" for localhost on local machine)
# Create a socket and bind the socket to the addr
HOST, PORT = "127.0.0.1", 12345

def listenToClient(conn, addr):
    try:
        conn.send(b"Welcom to the calculator server. Input your problem ?")

        while True:
            # Interact with the client
            request = conn.recv(1024).decode()
            if request == "exit()":
                conn.close()
                raise IOError
            else:
                operands_pattern = r"[+-/*//%^]+"
                operator = re.findall(operands_pattern, request.replace(" ", ""))
                operands = re.split(operands_pattern, request.replace(" ", ""))
                print(operator, operands)

                if len(operator) == 1 and len(operands) == 2:
                    try:
                        x, y = int(operands[0]), int(operands[1])
                        if operator[0] == "+":
                            conn.send((f"The answer is {x + y}").encode())
                        elif operator[0] == "-":
                            conn.send((f"The answer is {x - y}").encode())
                        elif operator[0] == "*":
                            conn.send((f"The answer is {x * y}").encode())
                        elif operator[0] == "/":
                            conn.send((f"The answer is {x / y}").encode())
                        # Bonus
                        elif operator[0] == "%":
                            conn.send((f"The answer is {x % y}").encode())
                        elif operator[0] == "^":
                            conn.send((f"The answer is {x ** y}").encode())
                        else:
                            conn.send(b"Invalid operator")
                    
                    except ValueError:
                        conn.send(b"Invalid operands")

                else:
                    conn.send(b"Invalid expression")

    except IOError:
        print(f"Client disconnected from {addr}")


with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as TCPServerSocket:
    print("TCP server socket successfully created!")

    TCPServerSocket.bind((HOST, PORT))
    print(f"TCP socket binds to {HOST}:{PORT}")

    TCPServerSocket.listen()
    print("Start to listen to clients")

    while(True):
        # Accept a new request and admit the connection
        conn, addr = TCPServerSocket.accept()
        print(f"Got the connection from {addr}")
        threading.Thread(target= listenToClient, args=(conn,addr)).start()
        