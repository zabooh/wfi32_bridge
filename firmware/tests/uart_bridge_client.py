import socket
import time

def main():
    host = '192.168.0.32'
    port = 47111

    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((host, port))

    try:
        while True:
            message = input("Enter a message to send to the server: ")
            client_socket.send(message.encode('utf-8'))
            time.sleep(0.1)
            response = client_socket.recv(1024)
            print(f"Received from server: {response.decode('utf-8')}")
    except KeyboardInterrupt:
        pass

    client_socket.close()

if __name__ == "__main__":
    main()
