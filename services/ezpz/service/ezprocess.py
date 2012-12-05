
import os
import traceback
import fcntl
from subprocess import *
import gevent, gevent.select

BUFSIZ = 16 * 1024
TIMEOUT = 10

def fdnonblock(fd):
	fcntl.fcntl(fd, fcntl.F_SETFL, fcntl.fcntl(fd, fcntl.F_GETFL) | os.O_NONBLOCK)

class ezprocess(object):
	def __init__(self, runfile, fns):
		self.runfile = runfile
		self.fns = fns
		self.output = ''
		self.errput = ''

	def run(self):
		p = Popen(["/usr/bin/python", "box.py", self.runfile], shell=False, bufsize=BUFSIZ, stdin=PIPE, stdout=PIPE, stderr=PIPE, close_fds=True)

		fdnonblock(p.stdin)
		fdnonblock(p.stdout)
		fdnonblock(p.stderr)

		with gevent.Timeout(TIMEOUT, False):
			while True:
				rfds, wfds, xfds = gevent.select.select([p.stdout, p.stderr], [], [], TIMEOUT-1)
				if p.stdout in rfds:
					tmp = p.stdout.read()
					self.output += tmp
					for line in tmp.split('\n'):
						self.handle_pipecommand(p, line)

				if p.stderr in rfds:
					tmp = p.stderr.read()
					self.errput += tmp

				if p.poll() != None: break

		try:
			p.stdin.close()
			self.output += p.stdout.read()
			self.errput += p.stderr.read()
		except:
			pass

		try:
			if p.poll() == None: p.terminate()
			p.stdout.close()
			p.stderr.close()
		except:
			pass

		return p.poll()

	def handle_pipecommand(self, p, line):
		if line.startswith('get '):
			cmd, key = line.split(' ', 1)
			key = key.strip()
			p.stdin.write('{0}\n'.format(self.fns.get(key)))
		elif line.startswith('close'):
			p.stdin.close()
		elif line.startswith('flush'):
			p.stdin.flush()
		elif line.startswith('put'):
			cmd, key = line.split(' ', 1)
			key = key.strip()
			try:
				content = open(self.errput.strip(), 'r').read()
				self.fns.put(key, content)
				p.stdin.write('put ok!')
			except:
				p.stdin.write('put fail!')
				traceback.print_exc()
