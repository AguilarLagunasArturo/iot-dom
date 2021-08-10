from http.server import BaseHTTPRequestHandler, HTTPServer
import time

hostName = "localhost"
serverPort = 8080

lines = []
device_uuid = ''

with open('index.html', 'r') as f:
    lines = f.read().split('\n')
    with open('uuid.md', 'r') as u:
        device_uuid = u.read().split('\n')[0]
    lines[61] = '''data: '{"action": "toggle-device-state", "device-id": "uuid"}','''
    lines[61] = lines[61].replace('uuid', device_uuid)
    lines[85] = '''<input type='text' value="uuid" disabled>'''
    lines[85] = lines[85].replace('uuid', device_uuid)

class MyServer(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        for l in lines:
            self.wfile.write(bytes(l, "utf-8"))

if __name__ == "__main__":
    webServer = HTTPServer((hostName, serverPort), MyServer)
    print("Server started http://%s:%s" % (hostName, serverPort))
    print("UUID: {}".format(device_uuid))

    try:
        webServer.serve_forever()
    except KeyboardInterrupt:
        pass

    webServer.server_close()
    print("Server stopped.")
