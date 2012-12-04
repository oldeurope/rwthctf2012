#ifndef TELNET_H
#define TELNET_H

int telnet_send_wrapper(int fd, const void *buf, size_t size);
int telnet_init(int fd);
int telnet_readchar(int fd, unsigned char *outchar, int timeoutMs);
int telnet_set_cursor(int fd, int x, int y);
int telnet_clear_screen(int fd);
int telnet_erase_current_line(int fd);
int telnet_erase_down(int fd);
int telnet_terminal_backspace(int fd);
int telnet_terminal_hidecursor(int fd);
int telnet_terminal_showcursor(int fd);

int telnet_get_term_height();
int telnet_get_term_width();

#endif
