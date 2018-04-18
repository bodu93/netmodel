import socket

if __name__ == "__main__":
    listening_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listening_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listening_socket.bind(('', 2018))
    listening_socket.listen(5)

    connected_socket, client_addr = listening_socket.accept()
    connected_socket.close() # send FIN
