import socket
import select
import struct

listen_address = ('', 2018)

if __name__ == "__main__":
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind(listen_address)
    server_socket.listen(socket.SOMAXCONN)
    server_socket.setblocking(0)
    poll = select.poll() # get a poller
    poll.register(server_socket.fileno(), select.POLLIN)
    print("server start listen at:", server_socket.getsockname())
    # print("server socket fileno:", server_socket.fileno())

    connections = {}
    while True:
        events = poll.poll(1000) # 10 seconds
        for fileno, event in events:
            # print("poll() returned with fileno:", fileno)
            if fileno == server_socket.fileno(): # listening socket
                client_socket, client_address = server_socket.accept()
                print("get connection from", client_address)
                #print("client socket", client_socket.fileno())
                # client_socket.setblocking(0)
                poll.register(client_socket.fileno(), select.POLLIN)
                connections[client_socket.fileno()] = client_socket
            # socket is readable
            # case 1: listening socket is readable
            # case 2: connected socket is readable
            elif event & select.POLLIN:
                # print("recv socket", fileno)
                client_socket = connections[fileno]
                data = client_socket.recv(4096)
                if data:
                    client_socket.sendall(data)
                else:
                    print("disconnect ", client_socket.getpeername())
                    poll.unregister(fileno)
                    client_socket.close()
                    del connections[fileno]
