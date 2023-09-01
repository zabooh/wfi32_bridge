from zeroconf import Zeroconf, ServiceBrowser
import socket

class TelnetServiceListener:
    def add_service(self, zeroconf, service_type, name):
        info = zeroconf.get_service_info(service_type, name)
        if info and "._tcp.local." in service_type:
            ip_addresses = [socket.inet_ntoa(addr) for addr in info.addresses]
            port = info.port
            hostname = info.server if info.server else "Unknown"
            
            print(f"Service found: {name}")
            print(f"  IP Addresses: {', '.join(ip_addresses)}")
            print(f"  Port: {port}")
            print(f"  Hostname: {hostname}")
            print("=" * 30)
            
    def update_service(self, zeroconf, type, name):
        pass            

def main():
    zeroconf = Zeroconf()
    listener_telnet = TelnetServiceListener()
    browser_telnet = ServiceBrowser(zeroconf, "_telnet._tcp.local.", listener_telnet)
    
    try:
        input("Press Enter to stop...")
    finally:
        zeroconf.close()

if __name__ == "__main__":
    main()
