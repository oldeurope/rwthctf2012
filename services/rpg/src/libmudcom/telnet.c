#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include "telnet.h"

// minimal telnet implementation

static const unsigned char cmd_will_echo[] = "\xFF\xFB\x01"; // WILL ECHO
static const unsigned char cmd_will_sup_go_ahead[] = "\xFF\xFB\x03"; // WILL SUPRESS GO AHEAD

static const unsigned char cmd_do_terminal[] = "\xFF\xFD\x18"; // DO terminal
static const unsigned char cmd_do_wsize[] = "\xFF\xFD\x1F"; // DO wsize

unsigned int telnet_echomode = 0;
int telnet_term_height = -1;
int telnet_term_width = -1;

int telnet_get_term_height() {
	return telnet_term_height;
}
int telnet_get_term_width() {
	return telnet_term_width;
}

int telnet_send_wrapper(int fd, const void *buf, size_t size) {
	return write(fd,buf,size);
}

int telnet_init(int fd) {
	// offer to echo
	return telnet_send_wrapper(fd,cmd_will_echo,sizeof(cmd_will_echo) -1);
}

static const unsigned char terminal_clear[] = "\x1B[2J";
int telnet_clear_screen(int fd) {
	return telnet_send_wrapper(fd, terminal_clear, sizeof(terminal_clear)-1);
}

int telnet_set_cursor(int fd, int x, int y) {
	char buf[128];
	snprintf(buf,sizeof(buf),"\x1B[%i;%iH",y+1,x+1);
	return telnet_send_wrapper(fd,buf,strlen(buf));
}

static const unsigned char terminal_erase_line[] = "\x1B[2K";
int telnet_erase_current_line(int fd) {
	return telnet_send_wrapper(fd, terminal_erase_line, sizeof(terminal_erase_line)-1);
}

static const unsigned char terminal_erase_last_char[] = "\x1B[D\x1B[K";
int telnet_terminal_backspace(int fd) {
	return telnet_send_wrapper(fd, terminal_erase_last_char, sizeof(terminal_erase_last_char)-1);
}

static const unsigned char terminal_hide_cursor[] = "\x1B[?25l";
int telnet_terminal_hidecursor(int fd) {
	return telnet_send_wrapper(fd, terminal_hide_cursor, sizeof(terminal_hide_cursor)-1);
}
static const unsigned char terminal_show_cursor[] = "\x1B[?25h";
int telnet_terminal_showcursor(int fd) {
	return telnet_send_wrapper(fd, terminal_show_cursor, sizeof(terminal_show_cursor)-1);
}

static const unsigned char terminal_erase_down[] = "\x1B[J";
int telnet_erase_down(int fd) {
	return telnet_send_wrapper(fd, terminal_erase_down, sizeof(terminal_erase_down)-1);
}

int telnet_readchar(int fd, unsigned char *outchar, int exitOnSpecial) {
	unsigned char c;
	do {
		size_t res = read(fd,&c,1);
		if (res <= 0)
			return -1;
		if (c == 0xFF) {
			// this is a telnet command - process it
			// read command type
			res = read(fd,&c,1);
			if (res <= 0)
				return -1;
			switch (c) {
				case 0xFA: // subcommand
					// what kind of subcommand is this?
					res = read(fd,&c,1);
					if (res <= 0)
						return -1;
					if (c == 0x1F) {
						// we negotiate about terminal size read with and height
						res = read(fd,&c,1);
						if (res <= 0)
							return -1;
						telnet_term_width = (unsigned int)c << 8;
						res = read(fd,&c,1);
						if (res <= 0)
							return -1;
						telnet_term_width |= c;
						res = read(fd,&c,1);
						if (res <= 0)
							return -1;
						telnet_term_height = (unsigned int)c << 8;
						res = read(fd,&c,1);
						if (res <= 0)
							return -1;
						telnet_term_height |= c;
					}
					// eat everything until end subcommand
					do {
						res = read(fd,&c,1);
						if (res <= 0)
							return -1;
						if (c == 0xFF) {
							res = read(fd,&c,1);
							if (res <= 0)
								return -1;
							if (c == 0xF0)
								break;
						}
					} while (1);
					if (exitOnSpecial & 1) {
						return 2;
					}
					break;
				case 0xFB: // WILL
					// read the parameter code
					res = read(fd,&c,1);
					if (res <= 0)
						return -1;
					switch (c) {
						case 0x18: // terminal type
							telnet_send_wrapper(fd, cmd_do_terminal, sizeof(cmd_do_terminal) -1);
							break;
						case 0x1F: // window size
							telnet_send_wrapper(fd, cmd_do_wsize, sizeof(cmd_do_wsize) -1);
							break;
					};
					break;
				case 0xFD: // DO
					res = read(fd,&c,1);
					if (res <= 0)
						return -1;
					switch (c) {
						case 0x01: // do echo
							telnet_echomode = 1;
//							printf("client ackknowleded echo mode\n");
							break;
						case 0x03: // supress go ahead
							// we will do this
							telnet_send_wrapper(fd, cmd_will_sup_go_ahead, sizeof(cmd_will_sup_go_ahead) - 1);
							break;
					}
					break;
				case 0xFF:
					*outchar = 0xFF;
					return 1;
					break;
			};
		}
		else {
			*outchar = c;
			if (c == '\r') {
				// carriage return is followed either by \0 or \n - however, kill it
				res = read(fd,&c,1);
				if (res <= 0)
					return -1;
			}
			return 1;
		}
	} while (1);
}
