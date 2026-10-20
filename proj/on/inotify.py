# wrapper for inotify syscall of linux
# otherwise I have to do busy poll loop

import os
from io import UnsupportedOperation
import ctypes
from dataclasses import dataclass

# taken from sys/inotify.h

# Supported events suitable for MASK parameter of INOTIFY_ADD_WATCH.
IN_ACCESS        = 0x00000001                          # File was accessed.
IN_MODIFY        = 0x00000002                          # File was modified.
IN_ATTRIB        = 0x00000004                          # Metadata changed.
IN_CLOSE_WRITE   = 0x00000008                          # Writtable file was closed.
IN_CLOSE_NOWRITE = 0x00000010                          # Unwrittable file closed.
IN_CLOSE         = (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE) # Close.
IN_OPEN          = 0x00000020                          # File was opened.
IN_MOVED_FROM    = 0x00000040                          # File was moved from X.
IN_MOVED_TO      = 0x00000080                          # File was moved to Y.
IN_MOVE          = (IN_MOVED_FROM | IN_MOVED_TO)       # Moves.
IN_CREATE        = 0x00000100                          # Subfile was created.
IN_DELETE        = 0x00000200                          # Subfile was deleted.
IN_DELETE_SELF   = 0x00000400                          # Self was deleted.
IN_MOVE_SELF     = 0x00000800                          # Self was moved.
# Events sent by the kernel.
IN_UNMOUNT    = 0x00002000 # Backing fs was unmounted.
IN_Q_OVERFLOW = 0x00004000 # Event queued overflowed.
IN_IGNORED    = 0x00008000 # File was ignored.
# Helper events.
IN_CLOSE = (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE) # Close.
IN_MOVE  = (IN_MOVED_FROM | IN_MOVED_TO)       # Moves.
# Special flags.
IN_ONLYDIR     = 0x01000000 # Only watch the path if it is a directory.
IN_DONT_FOLLOW = 0x02000000 # Do not follow a sym link.
IN_MASK_ADD    = 0x20000000 # Add to the mask of an already existing watch.
IN_ISDIR       = 0x40000000 # Event occurred against dir.
IN_ONESHOT     = 0x80000000 # Only send event once.

class _EventHeader(ctypes.Structure):
    _fields_ = [
        ("wd",     ctypes. c_int),
        ("mask",   ctypes.c_uint32),
        ("cookie", ctypes.c_uint32),
        ("len",    ctypes.c_uint32)
    ]

_HDR_SIZE = ctypes.sizeof(_EventHeader) 
assert _HDR_SIZE == 16

libc = ctypes.CDLL(None, use_errno=True)
inotify_init1     = libc.inotify_init1
inotify_add_watch = libc.inotify_add_watch
inotify_rm_watch  = libc.inotify_rm_watch
del libc

inotify_init1.argtypes     = [ctypes.c_int]
inotify_add_watch.argtypes = [ctypes.c_int, ctypes.c_char_p, ctypes.c_uint32]
inotify_rm_watch.argtypes  = [ctypes.c_int, ctypes.c_int]
inotify_init1.restype      =  ctypes.c_int
inotify_add_watch.restype  =  ctypes.c_int
inotify_rm_watch.restype   =  ctypes.c_int

def check_res(ret: int, path=None) -> int:
    if ret < 0:
        e = ctypes.get_errno()
        raise OSError(e, os.strerror(e), path)
    return ret

@dataclass(slots=True, kw_only=True)
class INotifyEvent:
    wd: int
    mask: int
    cookie: int
    name: bytes   # b"" when the event is about the watched item itself
    path: bytes   # the path that was passed to add_watch for this wd

class INotifyIO:
    def __init__(self, non_block=True):
        self.fd = check_res(inotify_init1(os.O_CLOEXEC | (os.O_NONBLOCK if non_block else 0)))
        self.paths: dict[int, bytes] = {}

    def fileno(self):
        return self.fd

    def add_watch(self, path, mask):
        path_bytes = os.fsencode(path)
        wd = check_res(inotify_add_watch(self.fd, path_bytes, mask), path)
        self.paths[wd] = path_bytes
        return wd

    def rm_watch(self, wd):
        check_res(inotify_rm_watch(self.fd, wd))

    def read(self, bufsize=65536):
        try:
            buf = os.read(self.fd, bufsize)
        except BlockingIOError:
            return []
        events = []
        pos = 0
        while pos < len(buf):
            hdr = _EventHeader.from_buffer_copy(buf, pos)
            pos += _HDR_SIZE
            name = buf[pos : pos+hdr.len].rstrip(b'\0')
            pos += hdr.len
            events.append(INotifyEvent(
                wd = hdr.wd,
                mask = hdr.mask,
                cookie = hdr.cookie,
                name = name,
                path = self.paths.get(hdr.wd, b'')
            ))
            if hdr.mask & IN_IGNORED:
                self.paths.pop(hdr.wd, None)
        return events

    def write(self, text_or_bytes):
        raise UnsupportedOperation("not writable")
    
    def close(self):
        if self.fd >= 0:
            os.close(self.fd)
            self.fd = -1

    def __enter__(self):
        return self

    def __exit__(self, *exc_info):
        self.close()
