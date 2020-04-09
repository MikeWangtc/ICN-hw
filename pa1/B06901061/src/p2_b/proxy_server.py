import socket
import time
import sys
import os
import threading



if len(sys.argv) <= 1:
	print('Usage : "python ProxyServer.py server_ip"\n[server_ip : It is the IP Address Of Proxy Server')
	sys.exit(2)
# "127.0.0.1"
HOST, PORT = sys.argv[1], 34566
serverAddressPort = ("localhost", 8081)

# handle request from client
def http_request_parser(request):
    method = request.decode().split()[0]
    filename = request.decode().split()[1].replace("?", "")[1:] # ignore absolute path symbol "?" and "/"
    
    filepath = filename                             # relative path
	
    print("===============Client request===============")
    print(f"Method: {method}")
    print(f"Target file: {filename}")
    # print(f"Request body: {request}")
    print(f"Serving web page: {filepath}")
    print("============================================")

    return filepath

# handle response from server
def http_response_parser(response):
	status_code = int(response.decode().split("\r\n")[0].split()[1])
	content = response.decode().split("\r\n")[-1]
	
	return status_code, content

def generate_response_client(status_code, data):
    header = ""

    if status_code == 200:
        header += 'HTTP/1.1 200 OK\r\n'
    elif status_code == 404:
        header += 'HTTP/1.1 404 Not Found\r\n'
    
    time_now = time.strftime("%a, %d %b %Y %H:%M:%S", time.localtime())
    header += f"Date: {time_now}\r\n"
    header += "Content-Type:text/html\r\n"
    header += 'Server: Proxy Server by Mike b06901061@NTUEE\r\n'
    header += 'Connection: close\r\n\r\n' # Signal that connection will be closed after completing the request
    
    response = header + data
    return response.encode()

def generate_request_server(file_path):
	request = f"GET /{file_path} HTTP/1.1\r\n"
	request += f"Host: {serverAddressPort[0]}\r\n"
	request +=  '''Connection: keep-alive\r\n\r\n
				'''

	return request.encode()

def listenToClient(conn, addr):
	# Receive request from the client
	http_request = conn.recv(1024)
	file_path = http_request_parser(http_request)
	
	response = ""
	try:
		# Check wether the file exist in the cache
		with open(file_path, 'r') as f:
			data = f.read()
			response = generate_response_client(200, data)

			print('Read from cache')
		
	# Error handling for file not found in cache
	except IOError:
		try:
			print("Requested file doesn't exist in cache")
			with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as TCPProxyClientSocket:
				TCPProxyClientSocket.connect(serverAddressPort)
				print(f"Connect to web server {serverAddressPort}")

				try:
					request = generate_request_server(file_path)
					TCPProxyClientSocket.send(request)
					print(f"Send request to {serverAddressPort}")
					response = TCPProxyClientSocket.recv(1024)
					print(f"Get response from {serverAddressPort}")
					
					status_code, content = http_response_parser(response)
					print(f"Status code: {status_code}")
					if status_code == 200:
						if file_path == "": file_path = "index.html"
						filename =file_path.split("/")[-1]
						with open(f"{filename}", 'w') as f:
							f.write(content)					
		
				except IOError:
					print("Illegal request")
		except socket.error:
			data = "<html><body><center><h1>Error 404: File not found</h1></center><p>Head back to <a href=\"/\">Redirect to main page</a>.</p></body></html>"
			response = generate_response_client(404, data)
			print(f"Fail to connect to web server {serverAddressPort}")

	conn.send(response)
	conn.close()
	print(f"Send response to client {addr}")

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as TCPProxyServerSocket:
	TCPProxyServerSocket.bind((HOST, PORT))
	TCPProxyServerSocket.listen()
	print(f"Proxy server socket is listening on: {HOST}:{PORT}")

	while True:
		# Strat receiving data from the client
		conn, addr = TCPProxyServerSocket.accept()
		print(f"Got the connection from {addr}")

		threading.Thread(target= listenToClient, args= (conn, addr)).start()

		
		

