#include "cache.h"

int copy_fd(int ifd, int ofd)
{
	while (1) {
		char buffer[8192];
		char *buf = buffer;
		ssize_t len = xread(ifd, buffer, sizeof(buffer));
		if (!len)
			break;
		if (len < 0) {
			int read_error;
			read_error = errno;
			close(ifd);
			return error("copy-fd: read returned %s",
				     strerror(read_error));
		}
		while (len) {
			int written = xwrite(ofd, buf, len);
			if (written > 0) {
				buf += written;
				len -= written;
			}
			else if (!written) {
				close(ifd);
				return error("copy-fd: write returned 0");
			} else {
				close(ifd);
				return error("copy-fd: write returned %s",
					     strerror(errno));
			}
		}
	}
	close(ifd);
	return 0;
}

int copy_file(const char *dst, const char *src, int mode)
{
	int fdi, fdo, status;

	mode = (mode & 0111) ? 0777 : 0666;
	if ((fdi = open(src, O_RDONLY)) < 0)
		return fdi;
	if ((fdo = open(dst, O_WRONLY | O_CREAT | O_EXCL, mode)) < 0) {
		close(fdi);
		return fdo;
	}
	status = copy_fd(fdi, fdo);
	if (close(fdo) != 0)
		return error("%s: write error: %s", dst, strerror(errno));

	if (!status && adjust_shared_perm(dst))
		return -1;

	return status;
}
