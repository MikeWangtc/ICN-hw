import socket
import time
import sys 
import os

def http_request_parser(request):
    method = request.decode().split()[0]
    filename = request.decode().split()[1].replace("?", "")[1:] # ignore absolute path symbol "?" and "/"
    
    if filename == "":
        filename = 'index.html'

    filepath = filename
    
    print("===============Client request===============")
    print(f"Method: {method}")
    print(f"Target file: {filename}")
    # print(f"Request body: {request}")
    print(f"Serving web page: {filepath}")
    print("============================================")

    return filepath

def generate_response(status_code, data):
    header = ""

    if status_code == 200:
        header += 'HTTP/1.1 200 OK\r\n'
    elif status_code == 404:
        header += 'HTTP/1.1 404 Not Found\r\n'
    
    time_now = time.strftime("%a, %d %b %Y %H:%M:%S", time.localtime())
    header += f"Date: {time_now}\r\n"
    header += "Content-Type:text/html\r\n"
    header += 'Server: Web Server by Mike b06901061@NTUEE\r\n'
    header += 'Connection: keep-Alive\r\n\r\n'
    
    response = header + data
    return response.encode()

HOST, PORT = "localhost", 8081

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as TCPServerSocket:
    TCPServerSocket.bind((HOST, PORT))
    TCPServerSocket.listen()
    print(f"Server socket is listening on: {HOST}:{PORT}")
    while True:

        conn, addr = TCPServerSocket.accept()
        print(f"Got the connection from {addr}")

        response = ""
        try:
            http_request = conn.recv(1024)
            file_path = http_request_parser(http_request)

            with open(file_path, 'r') as f:
                data = f.read()
                response = generate_response(200, data)
        
        except IOError:
            data = "<html><body><center><h1>Error 404: File not found</h1></center><p>Head back to <a href=\"/\">Redirect to main page</a>.</p></body></html>"
            response = generate_response(404, data)
        
        conn.send(response) 
        conn.close()
