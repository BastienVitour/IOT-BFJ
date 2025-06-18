import socket
import subprocess
import shlex
import threading
import time

seen_files = set()
pending_creates = {}  # filepath -> (timestamp, user, ip)
LISTENING_PORT = 8000
client_socket = None

CREATE_DELAY = 1.5  # seconds to wait before confirming a file was created

def handle_action(user, ip, filepath, action):
    global client_socket
    filename = filepath.split("/")[-1] if filepath else "unknown"
    message = f"{action.upper()}|{user}|{ip}|{filename}"
    print(f"📤 {message}")

    if client_socket:
        try:
            client_socket.sendall((message + "\n").encode())
        except Exception as e:
            print(f"⚠️ Lost connection to client: {e}")
            client_socket.close()
            client_socket = None

def socket_server():
    global client_socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind(("", LISTENING_PORT))
    server_socket.listen(1)
    print(f"🧠 Socket server waiting for one client on port {LISTENING_PORT}")

    while True:
        sock, addr = server_socket.accept()
        if client_socket is None:
            client_socket = sock
            print(f"✅ Client connected from {addr}")
        else:
            print(f"⛔ Extra client from {addr} rejected")
            sock.close()

def flush_pending_creates():
    while True:
        now = time.time()
        to_flush = []

        for filepath, (ts, user, ip) in list(pending_creates.items()):
            if now - ts >= CREATE_DELAY:
                to_flush.append(filepath)

        for filepath in to_flush:
            user, ip = pending_creates[filepath][1], pending_creates[filepath][2]
            del pending_creates[filepath]
            if filepath not in seen_files:
                seen_files.add(filepath)
                handle_action(user, ip, filepath, "create_file")

        time.sleep(0.5)


def watch_samba_logs():
    cmd = "journalctl -t smbd_audit -f -o cat"
    process = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE, text=True)

    for line in process.stdout:
        parts = line.strip().split('|')
        if len(parts) < 5:
            continue

        user = parts[0]
        ip = parts[1]
        action = parts[2]
        now = time.time()

        # -- CREATE_FILE (delay handling) --
        if action == "create_file" and len(parts) == 8:
            _, _, _, _, _, object_type, operation, filepath = parts

            if object_type != "file" or operation != "open":
                continue
            if any(skip in filepath for skip in ["/._", ".DS_Store", ".smbdelete"]):
                continue

            # Store as pending create
            pending_creates[filepath] = (now, user, ip)

        # -- UNLINKAT (cancel pending create if found) --
        elif action == "unlinkat" and len(parts) == 5:
            _, _, _, _, filepath = parts

            if any(skip in filepath for skip in ["/._", ".DS_Store", ".smbdelete"]):
                continue

            # If we were about to send a "create", cancel it
            if filepath in pending_creates:
                del pending_creates[filepath]

            if filepath in seen_files:
                seen_files.remove(filepath)

            handle_action(user, ip, filepath, "unlinkat")

# Start threads
threading.Thread(target=flush_pending_creates, daemon=True).start()
threading.Thread(target=watch_samba_logs, daemon=True).start()

# Start socket server in main thread
socket_server()