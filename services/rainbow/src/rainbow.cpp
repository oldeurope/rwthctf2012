#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <signal.h>

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <ctype.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <map>
#include <set>

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

typedef struct {
  int          iErrno;
  unsigned int uFunc;
} serror_t;

typedef enum {
  MENU_MAIN = 0,
  MENU_STORE,
  MENU_RETRIEVE,
  MENU_CONSOLE
} emenu_t;

typedef struct {
  emenu_t         eMenu;
  std::set<pid_t> setProc;
} state_t;

std::map<evutil_socket_t, state_t> g_mapClients;

const char g_cszMenu[] = "+--------------------------------------------------+\n"
                         "| rwthCTF digital vault - \x1B[31mrainbow edition\x1B[0m          |\n"
                         "+--------------------------------------------------+\n"
                         "| The following commands are available to you:     |\n"
                         "|                                                  |\n"
                         "| 1 - Store an item                                |\n"
                         "| 2 - Retrieve an item                             |\n"
                         "| 3 - Open developer console                       |\n"
                         "|                                                  |\n"
                         "+--------------------------------------------------+\n"
                         "> ";

const uid_t g_cuUidPriv   = 1000; // rainbow
const uid_t g_cuUidUnpriv = 1001; // shine

void menu_store(const char *cszInput, struct evbuffer *pOutput, std::map<evutil_socket_t, state_t>::iterator iterClient) {
  serror_t      sError   = { iErrno: 0, uFunc: 0 };
  unsigned char byKey[8];
  char          szBuffer[64];
  int           fdFile   = -1;
  int           iLen     = -1;
  ssize_t       iWritten = 0;

  for (const char *cszTmp = cszInput; *cszTmp; cszTmp++) {
    if (!isprint(*cszTmp) || (*cszTmp == '|')) {
      if (unlikely(evbuffer_add_printf(pOutput, "Information contains invalid characters. Please stick to printable characters.\n> ") == -1)) { sError.iErrno = 0; sError.uFunc = 1; goto error1; }
      return;
    }
  }

  evutil_secure_rng_get_bytes(byKey, sizeof(byKey));

  if (unlikely(snprintf(szBuffer, sizeof(szBuffer), "%02x%02x%02x%02x%02x%02x%02x%02x|%s\n", byKey[0], byKey[1], byKey[2], byKey[3], byKey[4], byKey[5], byKey[6], byKey[7], cszInput) < 0)) { sError.iErrno = 0; sError.uFunc = 2; goto error2; }

  if (unlikely((fdFile = open("keystore.txt", O_WRONLY | O_APPEND | O_NONBLOCK | O_CLOEXEC)) == -1)) { sError.iErrno = errno; sError.uFunc = 3; goto error3; }

  iLen = strlen(szBuffer);
  while (iWritten < iLen) {
    if (unlikely((iWritten = write(fdFile, &szBuffer[iWritten], iLen - iWritten)) == -1)) { sError.iErrno = errno; sError.uFunc = 4; goto error4; }
  }

  if (unlikely(close(fdFile))) { sError.iErrno = errno; sError.uFunc = 5; goto error5; }

  iterClient->second.eMenu = MENU_MAIN;
  if (unlikely(evbuffer_add_printf(pOutput, "Information stored. Your key is: %02x%02x%02x%02x%02x%02x%02x%02x\n> ", byKey[0], byKey[1], byKey[2], byKey[3], byKey[4], byKey[5], byKey[6], byKey[7]) == -1)) { sError.iErrno = 0; sError.uFunc = 6; goto error6; }
  return;

error6:
error5:
error4:
  if (sError.uFunc < 5) close(fdFile);
error3:
error2:
error1:
  fprintf(stderr, "error (menu_store): func=%u errno=%d\n", sError.uFunc, sError.iErrno);
}

//void menu_retrieve(const char *cszInput, struct evbuffer *pOutput, std::map<evutil_socket_t, state_t>::iterator iterClient) __attribute__ ((optimize("O0")));
void menu_retrieve(const char *cszInput, struct evbuffer *pOutput, std::map<evutil_socket_t, state_t>::iterator iterClient) {
  serror_t     sError  = { iErrno: 0, uFunc: 0 };
  int          fdFile  = -1;
  struct stat  sInfo;
  char        *szFile  = NULL;
  char        *szState = NULL;
  bool         bNotOk  = true; // this needs to be before szBuffer to allow overwriting via szBuffer
  char         szBuffer[64];
  int          iLen    = -1;

  if (unlikely((fdFile = open("keystore.txt", O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1)) { sError.iErrno = errno; sError.uFunc = 1; goto error1; }

  if (unlikely(fstat(fdFile, &sInfo))) { sError.iErrno = errno; sError.uFunc = 2; goto error2; }

  if (unlikely((szFile = (char *)mmap(NULL, sInfo.st_size + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_NONBLOCK, fdFile, 0)) == MAP_FAILED)) { sError.iErrno = errno; sError.uFunc = 3; goto error3; }

  szFile[sInfo.st_size] = '\0';

  if (unlikely(evbuffer_add_printf(pOutput, "Your access key matches the following entries:\n") == -1)) { sError.iErrno = 0; sError.uFunc = 4; goto error4; }

  for (char *szLine = strtok_r(szFile, "\n", &szState); szLine; szLine = strtok_r(NULL, "\n", &szState)) {
    char *szValue = strchr(szLine, '|');
    if (!szValue) continue;
    szValue++; // point to char after | -> but still to a valid location

    bNotOk = true;

    if (unlikely((iLen = snprintf(szBuffer, sizeof(szBuffer), "%s|%s", cszInput, szValue)) < 0)) { sError.iErrno = 0; sError.uFunc = 5; goto error5; }

    //szBuffer[iLen] = '\0'; // Vuln #1 -> iLen can be > sizeof(szBuffer) -> ausserdem ist der string bereits null-terminiert -> einfach raus nop-en

    if (!strcmp(szLine, szBuffer)) bNotOk = false;

    if (!bNotOk) {
      if (unlikely(evbuffer_add_printf(pOutput, "%s\n", szValue) == -1)) { sError.iErrno = 0; sError.uFunc = 6; goto error6; }
    }
  }

  if (unlikely(munmap(szFile, sInfo.st_size))) { sError.iErrno = errno; sError.uFunc = 7; goto error7; }

  if (unlikely(close(fdFile))) { sError.iErrno = errno; sError.uFunc = 8; goto error8; }

  iterClient->second.eMenu = MENU_MAIN;
  if (unlikely(evbuffer_add_printf(pOutput, "Finished listing entries.\n> ") == -1)) { sError.iErrno = 0; sError.uFunc = 9; goto error9; }
  return;

error9:
error8:
error7:
error6:
error5:
error4:
  if (sError.uFunc < 7) munmap(szFile, sInfo.st_size);
error3:
error2:
  if (sError.uFunc < 8) close(fdFile);
error1:
  fprintf(stderr, "error (menu_retrieve): func=%u errno=%d\n", sError.uFunc, sError.iErrno);
}

void menu_console(const char *cszInput, struct evbuffer *pOutput, std::map<evutil_socket_t, state_t>::iterator iterClient) {
  serror_t sError  = { iErrno: 0, uFunc: 0 };
  pid_t    pidProc = -1;
  long     lFlags  = 0;

  if (!strcmp(cszInput, "exit") || !strcmp(cszInput, "quit")) {
    if (!iterClient->second.setProc.empty()) {
      if (unlikely(evbuffer_add_printf(pOutput, "You have still %lu processes running. Please wait until they are finished.\n> ", iterClient->second.setProc.size()) == -1)) { sError.iErrno = 0; sError.uFunc = 1; goto error1; }
      return;
    }

    iterClient->second.eMenu = MENU_MAIN;
    if (unlikely(evbuffer_add_printf(pOutput, "> ") == -1)) { sError.iErrno = 0; sError.uFunc = 2; goto error2; }
    return;
  }

  if (unlikely((pidProc = fork()) == -1)) { sError.iErrno = errno; sError.uFunc = 3; goto error3; }

  if (!pidProc) {
    // child

    if (unlikely(dup2(iterClient->first, STDOUT_FILENO) == -1)) { sError.iErrno = errno; sError.uFunc = 4; goto error4; }
    if (unlikely(dup2(iterClient->first, STDERR_FILENO) == -1)) { sError.iErrno = errno; sError.uFunc = 5; goto error5; }

    if ((iterClient->first == STDOUT_FILENO) || (iterClient->first == STDERR_FILENO)) {
      if (unlikely((lFlags = fcntl(iterClient->first, F_GETFD)) == -1)) { sError.iErrno = errno; sError.uFunc = 6; goto error6; }
      if (unlikely(fcntl(iterClient->first, F_SETFD, lFlags & ~FD_CLOEXEC))) { sError.iErrno = errno; sError.uFunc = 7; goto error7; }
    }

    if (unlikely(setresuid(g_cuUidUnpriv, g_cuUidUnpriv, g_cuUidUnpriv))) { sError.iErrno = errno; sError.uFunc = 8; /* goto error8; */ } // Vuln #2

    if (unlikely(execl("/bin/sh", "sh", "-c", cszInput, NULL))) { sError.iErrno = errno; sError.uFunc = 9; goto error9; }
  }

  if (unlikely(!iterClient->second.setProc.insert(pidProc).second)) { sError.iErrno = 0; sError.uFunc = 10; goto error10; } // this COULD be an orphan if error4-10 occours
  return;

error10:
error9:
//error8:
error7:
error6:
error5:
error4:
  if (sError.uFunc < 10) exit(EXIT_FAILURE);
error3:
error2:
error1:
  fprintf(stderr, "error (menu_console): func=%u errno=%d\n", sError.uFunc, sError.iErrno);
}

void handle_read(struct bufferevent *pBuffer, void *pContext) {
  serror_t                                     sError   = { iErrno: 0, uFunc: 0 };
  struct evbuffer                             *pInput   = bufferevent_get_input(pBuffer);
  struct evbuffer                             *pOutput  = bufferevent_get_output(pBuffer);
  evutil_socket_t                              iSocket  = -1;
  std::map<evutil_socket_t, state_t>::iterator iterClient;
  char                                        *szInput  = NULL;
  unsigned long int                            ulInput  = 0;

  if (unlikely((iSocket = bufferevent_getfd(pBuffer)) == -1)) { sError.iErrno = 0; sError.uFunc = 1; goto error1; }
  if (unlikely((iterClient = g_mapClients.find(iSocket)) == g_mapClients.end())) { sError.iErrno = 0; sError.uFunc = 2; goto error2; }

  szInput = evbuffer_readln(pInput, NULL, EVBUFFER_EOL_CRLF);
  if (!szInput) return; // we dont have a complete line yet

  switch (iterClient->second.eMenu) {
    case MENU_MAIN:
      errno = 0;
      ulInput = strtoul(szInput, NULL, 10);
      if (unlikely(errno)) { sError.iErrno = errno; sError.uFunc = 3; goto error3; }

      switch (ulInput) {
        case MENU_STORE:
          iterClient->second.eMenu = MENU_STORE;
          if (unlikely(evbuffer_add_printf(pOutput, "Please enter the information you want to store:\n> ") == -1)) { sError.iErrno = 0; sError.uFunc = 4; goto error4; }
          break;
        case MENU_RETRIEVE:
          iterClient->second.eMenu = MENU_RETRIEVE;
          if (unlikely(evbuffer_add_printf(pOutput, "Please enter the access key:\n> ") == -1)) { sError.iErrno = 0; sError.uFunc = 5; goto error5; }
          break;
        case MENU_CONSOLE:
          iterClient->second.eMenu = MENU_CONSOLE;
          if (unlikely(evbuffer_add_printf(pOutput, "Please enter a console command:\n> ") == -1)) { sError.iErrno = 0; sError.uFunc = 6; goto error6; }
          break;
        default:
          if (unlikely(evbuffer_add_printf(pOutput, "Unknown command.\n> ") == -1)) { sError.iErrno = 0; sError.uFunc = 7; goto error7; }
          break;
      }
      break;
    case MENU_STORE:
      menu_store(szInput, pOutput, iterClient);
      break;
    case MENU_RETRIEVE:
      menu_retrieve(szInput, pOutput, iterClient);
      break;
    case MENU_CONSOLE:
      menu_console(szInput, pOutput, iterClient);
      break;
  }

  free(szInput);
  return;

error7:
error6:
error5:
error4:
error3:
  free(szInput);
error2:
error1:
  fprintf(stderr, "error (handle_read): func=%u errno=%d\n", sError.uFunc, sError.iErrno);
}

void handle_event(struct bufferevent *pBuffer, short sFlags, void *pContext) {
  serror_t                                     sError  = { iErrno: 0, uFunc: 0 };
  evutil_socket_t                              iSocket = -1;
  std::map<evutil_socket_t, state_t>::iterator iterClient;

  if (unlikely((iSocket = bufferevent_getfd(pBuffer)) == -1)) { sError.iErrno = 0; sError.uFunc = 1; goto error1; }

  if (sFlags & BEV_EVENT_ERROR) {
    fprintf(stderr, "error (handle_event): func=0 errno=%d\n", EVUTIL_SOCKET_ERROR());
  }

  if (sFlags & BEV_EVENT_EOF) {
    // with normal sockets, one could use getpeername() to get the ip address of the other side
    printf("debug: connection #%d closed\n", iSocket);
  }

  if (unlikely((iterClient = g_mapClients.find(iSocket)) == g_mapClients.end())) { sError.iErrno = 0; sError.uFunc = 2; goto error2; }

  for (std::set<pid_t>::iterator iterProc = iterClient->second.setProc.begin(); iterProc != iterClient->second.setProc.end(); iterProc++) {
    kill(*iterProc, SIGKILL); // dont check for a return value here, because we dont want to quit the loop because of a single erroneous process
    iterClient->second.setProc.erase(iterProc);
  }

  g_mapClients.erase(iterClient);

  bufferevent_free(pBuffer);
  return;

error2:
error1:
  bufferevent_free(pBuffer);
  fprintf(stderr, "error (handle_event): func=%u errno=%d\n", sError.uFunc, sError.iErrno);
}

void handle_connection(struct evconnlistener *pListener, evutil_socket_t iSocket, struct sockaddr *sAddr, int iAddrLen, void *pContext) {
  serror_t            sError  = { iErrno: 0, uFunc: 0 };
  struct event_base  *pBase   = evconnlistener_get_base(pListener);
  struct bufferevent *pBuffer = NULL;

  if (unlikely(evutil_make_socket_closeonexec(iSocket))) { sError.iErrno = 0; sError.uFunc = 1; goto error1; }

  // if BEV_OPT_THREADSAFE is given, this fails -> idk why
  // this can even be used with SSL -> bufferevent_openssl_socket_new()
  if (unlikely(!(pBuffer = bufferevent_socket_new(pBase, iSocket, BEV_OPT_CLOSE_ON_FREE)))) { sError.iErrno = 0; sError.uFunc = 2; goto error2; }
  bufferevent_setcb(pBuffer, handle_read, NULL, handle_event, NULL);
  if (unlikely(bufferevent_enable(pBuffer, EV_READ | EV_WRITE))) { sError.iErrno = 0; sError.uFunc = 3; goto error3; }

  if (unlikely(!g_mapClients.insert(std::pair<evutil_socket_t, state_t>(iSocket, { eMenu: MENU_MAIN })).second)) { sError.iErrno = 0; sError.uFunc = 4; goto error4; }

  printf("debug: connection #%d opened\n", iSocket);

  if (unlikely(bufferevent_write(pBuffer, g_cszMenu, strlen(g_cszMenu)))) { sError.iErrno = 0; sError.uFunc = 5; goto error5; }
  return;

error5:
  g_mapClients.erase(iSocket);
error4:
error3:
  bufferevent_free(pBuffer);
error2:
error1:
  if (sError.uFunc < 3) close(iSocket);
  fprintf(stderr, "error (handle_connection): func=%u errno=%d\n", sError.uFunc, sError.iErrno);
}

void handle_error(struct evconnlistener *pListener, void *pContext) {
  serror_t sError = { iErrno: 0, uFunc: 0 };

  fprintf(stderr, "error (handle_error): func=0 errno=%d\n", EVUTIL_SOCKET_ERROR());
//if (unlikely(event_base_loopexit(evconnlistener_get_base(pListener), NULL))) { sError.iErrno = 0; sError.uFunc = 1; goto error1; }
  return;

//error1:
  fprintf(stderr, "error (handle_error): func=%u errno=%d\n", sError.uFunc, sError.iErrno);
}

void handle_signal(evutil_socket_t iSignal, short sFlags, void *pContext) {
  serror_t           sError   = { iErrno: 0, uFunc: 0 };
  struct event_base *pBase    = (struct event_base *)pContext;
  pid_t              pidChild = -1;

  if (iSignal == SIGINT) {
    printf("debug: caught ctrl+c ... exiting ...\n");
    if (unlikely(event_base_loopexit(pBase, NULL))) { sError.iErrno = 0; sError.uFunc = 1; goto error1; }
  }
  else if (iSignal == SIGCHLD) {
    while ((pidChild = waitpid(-1, NULL, WNOHANG)) > 0) {
      for (std::map<evutil_socket_t, state_t>::iterator iterClient = g_mapClients.begin(); iterClient != g_mapClients.end(); iterClient++) {
        iterClient->second.setProc.erase(pidChild); // as this gets called for all clients, this fails often and only succeeds once
      }
    }
  }
  return;

error1:
  fprintf(stderr, "error (handle_signal): func=%u errno=%d\n", sError.uFunc, sError.iErrno);
}

int main(int iArgc, char *szArgv[]) {
  serror_t               sError      = { iErrno: 0, uFunc: 0 }; // equivalent to kernel's .iErrno=0 notation
  struct event_base     *pBase       = NULL;
  struct sockaddr_un     sAddr;
  const char             cszSocket[] = "/tmp/rainbow.sock";
  struct evconnlistener *pListener   = NULL;
  struct event          *pSigInt     = NULL;
  struct event          *pSigChld    = NULL;

  if (unlikely(setresuid(g_cuUidPriv, g_cuUidPriv, g_cuUidUnpriv))) { sError.iErrno = errno; sError.uFunc = 1; goto error1; }

  if (unlikely(evutil_secure_rng_init())) { sError.iErrno = 0; sError.uFunc = 2; goto error2; }

  if (unlikely(!(pBase = event_base_new()))) { sError.iErrno = 0; sError.uFunc = 3; goto error3; }

  memset(&sAddr, 0, sizeof(sAddr));
  sAddr.sun_family = AF_UNIX;
  snprintf(sAddr.sun_path, sizeof(sAddr.sun_path), cszSocket);
  if (unlikely(!(pListener = evconnlistener_new_bind(pBase, handle_connection, NULL, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE | LEV_OPT_THREADSAFE | LEV_OPT_CLOSE_ON_EXEC, -1, (struct sockaddr *)&sAddr, sizeof(sAddr))))) { sError.iErrno = 0; sError.uFunc = 4; goto error4; }
  evconnlistener_set_error_cb(pListener, handle_error);
  if (unlikely(chmod(cszSocket, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH))) { sError.iErrno = errno; sError.uFunc = 5; goto error5; }

  if (unlikely(!(pSigInt = evsignal_new(pBase, SIGINT, handle_signal, (void *)pBase)))) { sError.iErrno = 0; sError.uFunc = 6; goto error6; }
  if (unlikely(evsignal_add(pSigInt, NULL))) { sError.iErrno = 0; sError.uFunc = 7; goto error7; }

  if (unlikely(!(pSigChld = evsignal_new(pBase, SIGCHLD, handle_signal, (void *)pBase)))) { sError.iErrno = 0; sError.uFunc = 8; goto error8; }
  if (unlikely(evsignal_add(pSigChld, NULL))) { sError.iErrno = 0; sError.uFunc = 9; goto error9; }

  printf("debug: ready to receive connections\n");
  if (unlikely(event_base_dispatch(pBase))) { sError.iErrno = 0; sError.uFunc = 10; goto error10; }

  for (std::map<evutil_socket_t, state_t>::iterator iterClient = g_mapClients.begin(); iterClient != g_mapClients.end(); iterClient++) {
    for (std::set<pid_t>::iterator iterProc = iterClient->second.setProc.begin(); iterProc != iterClient->second.setProc.end(); iterProc++) {
      kill(*iterProc, SIGKILL);
      iterClient->second.setProc.erase(iterProc);
    }
    g_mapClients.erase(iterClient);
  }

  event_free(pSigChld);

  event_free(pSigInt);

  evconnlistener_free(pListener);
  if (unlikely(unlink(cszSocket))) { sError.iErrno = errno; sError.uFunc = 11; goto error11; }

  event_base_free(pBase);

  printf("debug: all work done\n");
  return EXIT_SUCCESS;

error11:
error10:
error9:
  if (sError.uFunc < 11) event_free(pSigChld);
error8:
error7:
  if (sError.uFunc < 11) event_free(pSigInt);
error6:
error5:
  if (sError.uFunc < 11) evconnlistener_free(pListener);
  if (sError.uFunc < 11) unlink(cszSocket);
error4:
  event_base_free(pBase);
error3:
error2:
error1:
  fprintf(stderr, "error (main): func=%u errno=%d\n", sError.uFunc, sError.iErrno);
  return EXIT_FAILURE;
}

/*
  # apt-get install libevent-dev
  $ g++ rwthctf.cpp -o rwthctf -Wall -levent -O3 -s -flto -fwhole-program -std=c++11
  $ strip --remove-section=.note.ABI-tag      rwthctf -> "for GNU/Linux 2.6.26"
  $ strip --remove-section=.note.gnu.build-id rwthctf -> "BuildID[sha1]=0x9530320f9fdf1f2a533d40cede9d669a44d8d3d9"
*/
