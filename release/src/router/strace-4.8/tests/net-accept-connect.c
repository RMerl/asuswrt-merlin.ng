#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SUN_PATH "local-stream"
int main(void)
{
	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
		.sun_path = SUN_PATH
	};
	socklen_t len = offsetof(struct sockaddr_un, sun_path) + sizeof SUN_PATH;

	unlink(SUN_PATH);
	close(0);
	close(1);

	assert(socket(PF_LOCAL, SOCK_STREAM, 0) == 0);
	assert(bind(0, (struct sockaddr *) &addr, len) == 0);
	assert(listen(0, 5) == 0);

	memset(&addr, 0, sizeof addr);
	assert(getsockname(0, (struct sockaddr *) &addr, &len) == 0);

	pid_t pid = fork();
	assert(pid >= 0);

	if (pid) {
		assert(accept(0, (struct sockaddr *) &addr, &len) == 1);
		assert(close(0) == 0);
		int status;
		assert(waitpid(pid, &status, 0) == pid);
		assert(status == 0);
		assert(close(1) == 0);
	} else {
		assert(socket(PF_LOCAL, SOCK_STREAM, 0) == 1);
		assert(close(0) == 0);
		assert(connect(1, (struct sockaddr *) &addr, len) == 0);
		assert(close(1) == 0);
		return 0;
	}

	unlink(SUN_PATH);
	return 0;
}
