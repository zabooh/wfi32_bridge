import telnetlib
import sys
import threading

class TelnetSession:
    def __init__(self, host, port):
        self.tn = telnetlib.Telnet(host, port)
        self.running = True
        self.lock = threading.Lock()

        # Starte einen Thread für die Ausgabe des Telnet-Servers
        self.output_thread = threading.Thread(target=self.receive_output)
        self.output_thread.start()

    def receive_output(self):
        while self.running:
            try:
                data = self.tn.read_some()
                if data:
                    sys.stdout.write(data.decode("utf-8"))
                    sys.stdout.flush()
            except Exception as e:
                print("Fehler beim Empfangen der Ausgabe:", e)
                break

    def send_input(self):
        try:
            while self.running:
                user_input = input()
                if not user_input:
                    break

                with self.lock:
                    self.tn.write(user_input.encode("utf-8") + b"\n")
        except Exception as e:
            print("Fehler beim Senden der Eingabe:", e)

    def start(self):
        try:
            self.send_input_thread = threading.Thread(target=self.send_input)
            self.send_input_thread.start()
            self.send_input_thread.join()
        except KeyboardInterrupt:
            pass
        finally:
            self.running = False
            self.output_thread.join()
            self.tn.close()

if __name__ == "__main__":
    host = "192.168.0.32"
    port = 23  # Standard-Telnet-Port

    telnet_session = TelnetSession(host, port)
    telnet_session.start()
