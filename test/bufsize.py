import socket

if __name__ == "__main__":
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    snd_buflen = udp_socket.getsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF)
    print("udp's default send buf length is:", snd_buflen)
    rcv_buflen = udp_socket.getsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF)
    print("udp's default recv buf length is:", rcv_buflen)

    tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    snd_buflen = tcp_socket.getsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF)
    print("tcp's default send buf length is:", snd_buflen)
    rcv_buflen = tcp_socket.getsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF)
    print("tcp's default recv buf length is:", rcv_buflen)
