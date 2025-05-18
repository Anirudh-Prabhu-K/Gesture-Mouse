import socket
import pyautogui

# Configuration
UDP_IP = "0.0.0.0"      # Listen on all network interfaces
UDP_PORT = 4210         # Port to listen on
SPEED_MULTIPLIER = 50  # Increase to make pointer move faster

# Create UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Listening for UDP packets on port {UDP_PORT}...")

while True:
    data, addr = sock.recvfrom(1024)
    message = data.decode().strip()

    if message == "CLICK":
        pyautogui.click()
        print("Click!")
    else:
        try:
            dx, dy = map(float, message.split(","))
            pyautogui.moveRel(dx * SPEED_MULTIPLIER, dy * SPEED_MULTIPLIER)
        except Exception as e:
            print(f"Invalid data: {message} | Error: {e}")
