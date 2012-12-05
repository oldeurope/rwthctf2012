import sys
import traceback
import os
import socket
import imp

from seccomp import *
f = SyscallFilter(defaction=KILL)
f.add_rule(ALLOW, "open")
f.add_rule(ALLOW, "close")
f.add_rule(ALLOW, "read")
f.add_rule(ALLOW, "write")
f.add_rule(ALLOW, "lseek", Arg(0, EQ, sys.stdout))
f.add_rule(ALLOW, "rt_sigreturn")
f.add_rule(ALLOW, "rt_sigaction")
f.add_rule(ALLOW, "rt_sigprocmask")
f.add_rule(ALLOW, "exit")
f.add_rule(ALLOW, "select")
f.add_rule(ALLOW, "socket")
f.add_rule(ALLOW, "socketcall")
f.add_rule(ALLOW, "ioctl")
f.add_rule(ALLOW, "fstat")
f.add_rule(ALLOW, "getdents64")
f.add_rule(ALLOW, "fstat64")
f.add_rule(ALLOW, "fcntl64")
f.add_rule(ALLOW, "stat64")
f.add_rule(ALLOW, "_llseek")
f.add_rule(ALLOW, "brk")
f.add_rule(ALLOW, "mmap")
f.add_rule(ALLOW, "munmap")
f.add_rule(ALLOW, "time")
f.add_rule(ALLOW, "unlink")
f.add_rule(ALLOW, "fcntl")
f.add_rule(ALLOW, "nanosleep")
f.add_rule(ALLOW, "_newselect")
f.load()
logic = imp.load_module('logic', open(sys.argv[1], 'U'), '.', ('.py', 'U', 1))
