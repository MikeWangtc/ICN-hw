import socket

serverAddressPort = ("127.0.0.1", 12345)
# TA server
# serverAddressPort = ("140.112.42.100", 7777)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as TCPClientSocket:
    print("Successfully create a TCP client socket")

    TCPClientSocket.connect(serverAddressPort)
    print(f"Connect to server {serverAddressPort}")

    print(TCPClientSocket.recv(1024).decode())
    print("If you want to terminate the process, please enter exit()")

    while True:
        try:
            request = input().strip()
            if request == "exit()" or request == "":
                # inform server to drop out the connection before disconnection
                # prevent broken pipe error
                TCPClientSocket.send(b"exit()")
                print(TCPClientSocket.recv(1024).decode())
                raise IOError
            else:
                TCPClientSocket.send(request.encode())
                response = TCPClientSocket.recv(1024).decode()
                print(f"Receive server message:")
                print(response)

        except IOError:
            print("Thanks for using the calculator")
            break

