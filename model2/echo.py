from socketserver import BaseRequestHandler, TCPServer
from socketserver import ForkingTCPServer, ThreadingTCPServer

class EchoHandler(BaseRequestHandler):
    def handle(self):
        print("got connection from", self.client_address)
        while True:
            data = self.request.recv(4096)
            if data:
                sent = self.request.sendall(data)
            else:
                print("disconnect", self.client_address)
                self.request.close()
                break

if __name__ == "__main__":
    listen_address = ("0.0.0.0", 2007)
    server = ThreadingTCPServer(listen_address, EchoHandler)
    server.serve_forever()
