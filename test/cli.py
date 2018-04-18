import socket, time

if __name__ == "__main__":
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect(('', 2018))

    time.sleep(5) # waiting server for send FIN

    data = b'data'
    l = client_socket.sendall(data) # let server send RST
    print(l)
    # that is:
    # server sent FIN and RST,
    # and now, client read this socket, recv() will return -1 and ECONNRESET
    # (that is, in tcp protocol stack codes, RST cover FIN segment in TCP stack)
    data = client_socket.recv(1024)
    print(len(data))
    # l = client_socket.sendall(data)
    # print(l)
