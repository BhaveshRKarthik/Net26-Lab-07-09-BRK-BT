from ConfigFile import read_config
import inotify
import select
import socket
import struct
import pathlib
from dataclasses import dataclass, field
from consts import *

@dataclass(slots=True)
class VirtualNode:
    sock: socket.socket
    letter: bytes = field(default=b'@')

    # raw 6-byte CONNECT body (LINK-STATE has same 6 bytes required in same order)
    connect: bytearray = field(default_factory=bytearray)  
 
 
class OracleServer:
    def __init__(self, host_ip: str, port: int, config_path: pathlib.Path):
        self.path = config_path
        self.config = read_config(self.path, None)
        self.n = len(self.config)
 
        self.clients: dict[int, VirtualNode] = {} # all connections
        self.nodes : list[VirtualNode] = [] # actual nodes (non-extra)
 
        self.ep = select.epoll()
        
        server = self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server.bind((host_ip, port))
        server.listen()

        self.ino = inotify.INotifyIO()
        self.ino.add_watch(self.path.parent, inotify.IN_CLOSE_WRITE | inotify.IN_MOVED_TO)
        self.ep.register(self.server.fileno(), select.EPOLLIN)
        self.ep.register(self.ino.fileno(), select.EPOLLIN)
 
    def __enter__(self):
        return self
 
    def __exit__(self, exc_type, exc_val, exc_tb):
        if exc_type is not None:
            print(f"\n{RED}Exception Encountered:{RESET}\n")

        for c in self.clients.values():
            c.sock.close()
        for x in (self.server, self.ino, self.ep):
            x.close()
 
    def run(self):
        while True:
            for fd, _ in self.ep.poll():
                if fd == self.server.fileno():
                    self.accept()
                elif fd == self.ino.fileno():
                    self.config_event()
                else:
                    self.read_connect(self.clients[fd])
 
    def accept(self):
        sock, _ = self.server.accept()
        sock.setblocking(False)
        c = VirtualNode(sock)
        self.clients[sock.fileno()] = c
        if len(self.nodes) < self.n:
            c.letter = bytes([65+len(self.nodes)])
            print(f"{GREEN}[+] Accepted connect from {c.letter.decode()}{RESET}: {sock.getpeername()[0]}:{sock.getpeername()[1]}")
            self.nodes.append(c)
            self.ep.register(sock.fileno(), select.EPOLLIN)
 
    def read_connect(self, c: VirtualNode):
        if len(c.connect) == 6:
            raise ConnectionError("VN disconnected or sent unexpected data")
        try:
            data = c.sock.recv(6 - len(c.connect))
        except BlockingIOError:
            return
        c.connect += data
        # disconnect or CONNECT complete
        if not data:
            raise ConnectionError("VN disconnected before sending CONNECT")
        if len(c.connect) == 6:
            print(f"{GREEN}[+] Received CONNECT from {c.letter.decode()}{RESET}: {c.connect.hex('-').upper()}")
            c.sock.setblocking(True)
            if len(self.nodes) == self.n and all(len(x.connect) == 6 for x in self.nodes):
                print(f"\n{GREEN}ALL VIRTUAL NODES CONNECTED{RESET}")
                self.broadcast()

 
    def config_event(self):
        name = self.path.name.encode()
        if not any(e.name == name for e in self.ino.read()):
            return
        try:
            new = read_config(self.path, self.n)
        except (AssertionError, ValueError, OSError):
            print(f"{RED}WARNING: {YELLOW}half written/invalid config detected. continuing with old config.{RESET}")
            return  # half-written or invalid file: keep the old config
        if new != self.config:
            self.config = new
            print(f"\n{GREEN}Virtual Node topology change detected{RESET}")
            self.broadcast()
 
    def broadcast(self):
        for i, me in enumerate(self.nodes):
            msg = bytes()
            for j, peer in enumerate(self.nodes):
                if self.config[i][j] != -1:
                    msg += peer.letter + peer.connect + struct.pack("!I", self.config[i][j])
            me.sock.sendall(bytes([len(msg)//11]) + msg)
            print(f"{BLUE}  -> Sent LINK-STATE to {me.letter.decode()}{RESET}")
 