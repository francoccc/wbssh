// wbssh.cpp: 定义应用程序的入口点。
//

#include "wbssh.h"

#include <libssh2.h>

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#include <poll.h>
#endif

#include "include/poll.hpp"

using namespace std;

#define PASSWORD 1
#define KEYBORAD_INTERACTIVE 2
#define PUBLIC_KEY 4

#define EXIT_COMMAND "exit"

int IPStringToNumber(const char *cip, uint32_t *uip) {
	uint32_t byte0;
	uint32_t byte1;
	uint32_t byte2;
	uint32_t byte3;
	char dummy[2];
	if (sscanf(cip, "%u.%u.%u.%u%1s", &byte3, &byte2, &byte1, &byte0, dummy) == 4) {
		if (byte0 < 256 && byte1 < 256 && byte2 < 256 && byte3 < 256) {
			*uip = (byte3 << 24) + (byte2 << 16) + (byte1 << 8) + byte0;
			return 0;
		}
	}
	return -1;
}

static const char* pubkey = ".ssh/id_rsa.pem.pub";
static const char* privkey = ".ssh/id_rsa.pem";
static const char* username = "root";
static const char* password = "password";


LIBSSH2_SESSION* session = NULL;
LIBSSH2_CHANNEL* channel = NULL;
libssh2_socket_t sock;

DWORD WINAPI main_loop(LPVOID lpParam) {
	const int numfds = 2;
	char commandbuf[0x4000];
	size_t written{ 0 };
	int rc;

	pollfd pfds[numfds];
	memset(pfds, 0, numfds * sizeof(pollfd));

	// loop
	do {
		pfds[0].fd = sock;
		pfds[0].events = POLLIN;
		pfds[0].revents = 0;
		pfds[1].fd = _fileno(stdin);
		pfds[1].events = POLLIN;
		pfds[1].revents = 0;

		/* Polling on socket and stdin while we are
		 * not ready to read from it */
		rc = poll(pfds, numfds, -1);
		if (-1 == rc) {
			perror("poll");
			break;
		}

		if (pfds[0].revents & POLLIN) {
			/* Read output from remote side */
			char inputbuf[BUFSIZ];
			do {
				rc = libssh2_channel_read(channel, inputbuf, BUFSIZ);
				if (rc > 0) {
					fwrite(inputbuf, 1, rc, stdout);
					memset(inputbuf, 0, rc);
				}
			} while (LIBSSH2_ERROR_EAGAIN != rc && rc > 0);
		}
		if (rc < 0 && LIBSSH2_ERROR_EAGAIN != rc) {
			fprintf(stderr, "libssh2_channel_read error code %d\n", rc);
			return (EXIT_FAILURE);
		}

		if (pfds[1].revents & POLLIN) {
			/* Request for command input */
			fgets(commandbuf, BUFSIZ - 2, stdin);
			if (strcmp(commandbuf, EXIT_COMMAND) == 0)
				break;

			/* Adjust command format */
			commandbuf[strlen(commandbuf) - 1] = '\r';
			commandbuf[strlen(commandbuf)] = '\n';
			commandbuf[strlen(commandbuf) + 1] = '\0';

			/* Write command to stdin of remote shell */
			written = 0;
			do {
				rc = libssh2_channel_write(channel, commandbuf, strlen(commandbuf));
				written += rc;
			} while (LIBSSH2_ERROR_EAGAIN != rc
				&& rc > 0
				&& written != strlen(commandbuf));
			memset(commandbuf, 0, BUFSIZ);
		}
		if (rc < 0 && LIBSSH2_ERROR_EAGAIN != rc) {
			fprintf(stderr, "libssh2_channel_write error code %d\n", rc);
			return (EXIT_FAILURE);
		}

	} while (1); 
}

int main(int argc, char** argv) {
	uint32_t hostaddr;
	int rc, auth_pw = 0;
	char* h;
	struct sockaddr_in sin;
	const char* fingerprint;
	char* userauthlist;
	size_t rd = 0;

#ifdef WIN32
	h = getenv("USERPROFILE");
#else
	h = getenv("HOME");
#endif
	if (argc > 1) {
		hostaddr = inet_addr(argv[1]);
	}
	else {
		hostaddr = htonl(0x7F000001);		// 127.0.0.1
	}

	if (argc > 2) {
		username = argv[2];
	}

	if (argc > 3) {
		password = argv[3];
	}

#ifdef WIN32
	WSADATA wsadata;
	rc = WSAStartup(MAKEWORD(2, 0), &wsadata);
	if (rc) {
		fprintf(stderr, "WSAStartup failed with error: %d\n", rc);
		return 1;
	}
#endif
	rc = libssh2_init(0);

	if (rc) {
		fprintf(stderr, "libssh2 initialization failed : (%d)\n", rc);
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == LIBSSH2_INVALID_SOCKET) {
		fprintf(stderr, "failed to create socket\n");
		rc = 1;
		goto shutdown;
	}
	sin.sin_family = AF_INET;
	sin.sin_port = htons(22);
	sin.sin_addr.s_addr = hostaddr;

	fprintf(stderr, "connect to %s:%d user: %s\n", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port), username);
	if (connect(sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in))) {
		fprintf(stderr, "failed to connect\n");
		rc = 1;
		goto shutdown;
	}
	session = libssh2_session_init();

	if (!session) {
		fprintf(stderr, "Could not initialize SSH session\n");
		rc = 2;
		goto shutdown;
	}
	//libssh2_trace(session, ~0);

	rc = libssh2_session_handshake(session, sock);
	if (rc) {
		fprintf(stderr, "Failure establishing SSH session: %d\n", rc);
		goto shutdown;
	}

	rc = 1;

	fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
	fprintf(stderr, "Fingerprint: ");
	for (int i = 0; i < 20; i++) {
		fprintf(stderr, "%02X", (unsigned char)fingerprint[i]);
	}
	fprintf(stderr, "\n");

	userauthlist = libssh2_userauth_list(session, username, strlen(username));
	if (userauthlist) {
		if (strstr(userauthlist, "password")) {
			auth_pw |= PASSWORD;
		}
		if (strstr(userauthlist, "keyboard-interactive")) {
			auth_pw |= KEYBORAD_INTERACTIVE;
		}
		if (strstr(userauthlist, "publickey")) {
			auth_pw |= PUBLIC_KEY;
		}
	}

	if (auth_pw & 4) {
		fprintf(stderr, "Auth by public key\n");
		char* fn1, * fn2;
		size_t fn1sz, fn2sz;
		fn1sz = strlen(h) + strlen(pubkey) + 2;
		fn2sz = strlen(h) + strlen(privkey) + 2;
		fn1 = (char*)malloc(fn1sz);
		fn2 = (char*)malloc(fn2sz);
		if (!fn1 || !fn2) {
			fprintf(stderr, "out of memory\n");
			free(fn1);
			free(fn2);
			goto shutdown;
		}
		snprintf(fn1, fn1sz, "%s/%s", h, pubkey);
		snprintf(fn2, fn2sz, "%s/%s", h, privkey);

		if (access(fn2, F_OK) == 0) {
			fprintf(stderr, "Found file:  %s\n", fn2);
		}

		if (libssh2_userauth_publickey_fromfile(session, username, fn1, fn2, "")) {
			//if (libssh2_userauth_password(session, username, password)) {
			fprintf(stderr, "Authentication by publickey failed %s\n", fn1);
			goto shutdown;
		}
		else {
			fprintf(stderr, "Authenticated to %s ([%s]:%d)\n", inet_ntoa(sin.sin_addr), inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
		}
		free(fn1);
		free(fn2);
	}


#if 0 
	if (libssl2_channel_request_ptr(channel, "vanilla")) {

	}
#endif
#if 0
	if (libssh2_channel_exec(channel, argv[5])) {}
#endif
	channel = libssh2_channel_open_session(session);

	if (!channel) {
		fprintf(stderr, "failed to open a session\n");
		goto shutdown;
	}

	if (libssh2_channel_request_pty(channel, "vanilla")) {
		fprintf(stderr, "Failed requesting pty\n");
		goto shutdown;
	}

	if (libssh2_channel_shell(channel)) {
		fprintf(stderr, "Unable to request shell on allocated pty\n");
		goto shutdown;
	}

	libssh2_channel_set_blocking(channel, 0);

	printf("atty: %d\n", isatty(_fileno(stdin)));

#ifdef WIN32
	HANDLE threads[1];
	threads[0] = CreateThread(NULL, 0, main_loop, NULL, 0, NULL);
	// 等待所有线程结束
	for (;;);
	WaitForMultipleObjects(1, threads, TRUE, INFINITE);
	CloseHandle(threads[0]);
#else
	char buf[1024];
	rd= libssh2_channel_read(channel, buf, sizeof(buf));
	if (rd < 0)
		fprintf(stderr, "Unable to read response: %d\n", int(rd));
	else
		fwrite(buf, 1, rd, stdout);


	while (!libssh2_channel_eof(channel)) {
		rd = libssh2_channel_read(channel, buf, sizeof(buf));
		if (rd < 0)
			fprintf(stderr, "Unable to read response: %d\n", int(rd));
		else {
			fwrite(buf, 1, rd, stdout);
		}
		//libssh2_channel_write(channel, command, strlen(command));
		fprintf(stdout, ">| ");
		char command[1024];
		fgets(command, sizeof(command), stdin);
		if (strstr(command, "exit")) {
			break;
		}
		size_t command_len = strlen(command);
		if ((rc = libssh2_channel_write(channel, command, command_len)) < 0) {
			fprintf(stderr, "exec error\n");
			goto shutdown;
		}
	}
#endif


	fprintf(stderr, "Exit interactive\n");

	rc = libssh2_channel_get_exit_status(channel);
	if (libssh2_channel_close(channel)) {
		fprintf(stderr, "Unable to close channel\n");
	}

	if (channel) {
		libssh2_channel_free(channel);
		channel = NULL;
	}


shutdown:
	if (session) {
		libssh2_session_disconnect(session, "Normal Shutdown");
		libssh2_session_free(session);
	}

	if (sock != LIBSSH2_INVALID_SOCKET) {
		shutdown(sock, 2);
#ifdef WIN32
		closesocket(sock);
#else
		close(sock);
#endif
	}

	libssh2_exit();

	fprintf(stderr, "all done\n");
	return rc;
}
