from socketserver import BaseRequestHandler, TCPServer

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

listen_address = ('', 2018)
if __name__ == "__main__":
    server = TCPServer(listen_address, EchoHandler)
    server.serve_forever()
