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
    # raw 6-byte CONNECT body (LINK-STATE has same 6 bytes required in same order)
    connect: bytearray | None = None  
 
 
class OracleServer:
    def __init__(self, host_ip: str, port: int, config_path: pathlib.Path):
        self.path = config_path.resolve()
        self.config = read_config(self.path, None)
        self.n = len(self.config)
 
        self.clients: dict[int, VirtualNode] = {} # all connections
        self.nodes = [] # actual nodes (non-extra)
 
        self.ep = select.epoll()
        self.server = socket.create_server((host_ip, port))
        self.ino = inotify.INotifyIO()
        self.ino.add_watch(self.path.parent, inotify.IN_CLOSE_WRITE | inotify.IN_MOVED_TO)
        self.ep.register(self.server.fileno(), select.EPOLLIN)
        self.ep.register(self.ino.fileno(), select.EPOLLIN)
 
    def __enter__(self):
        return self
 
    def __exit__(self, *exc_info):
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
            self.nodes.append(c)
            self.ep.register(sock.fileno(), select.EPOLLIN)
 
    def read_connect(self, c):
        try:
            data = c.sock.recv(6 - len(c.connect))
        except BlockingIOError:
            return
        c.connect += data
        # disconnect or CONNECT complete
        if len(c.connect) == 6 or not data:
            self.ep.unregister(c.sock.fileno())
        if len(c.connect) == 6:
            c.sock.setblocking(True)
            self.broadcast()
        else:
            print(f'{RED}Warning:{RESET} {YELLOW}Virtual Node Disconnected.{RESET}')

 
    def config_event(self):
        name = self.path.name.encode()
        if not any(e.name == name for e in self.ino.read()):
            return
        try:
            new = read_config(self.path, self.n)
        except (AssertionError, ValueError, OSError):
            return  # half-written or invalid file: keep the old config
        if new != self.config:
            self.config = new
            self.broadcast()
 
    def broadcast(self):
        if len(self.nodes) < self.n or any(c.connect is None for c in self.nodes):
            return
        for i, me in enumerate(self.nodes):
            msg = bytes([self.n])
            for j, peer in enumerate(self.nodes):
                cost = self.config[i][j] & 0xFFFFFFFF # limit to 32 bits
                msg += bytes([65 + j]) + peer.connect + struct.pack("!I", cost)
            try:
                me.sock.sendall(msg)
            except OSError:
                pass
 