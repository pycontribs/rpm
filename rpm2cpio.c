/* rpmarchive: spit out the main archive portion of a package */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <zlib.h>

#include "lib/package.h"
#include "rpmlib.h"

int main(int argc, char **argv)
{
    int fd;
    Header hd;
    int rc, isSource;
    char buffer[1024];
    int ct;
    gzFile stream;
    
    if (argc == 1) {
	fd = 0;
    } else {
	fd = open(argv[1], O_RDONLY, 0644);
    }

    if (fd < 0) {
	perror("cannot open package");
	exit(1);
    }

    rc = pkgReadHeader(fd, &hd, &isSource);
    if (rc == 1) {
	fprintf(stderr, "argument is not an RPM package\n");
	exit(1);
    } else if (rc) {
	fprintf(stderr, "error reading header from package\n");
	exit(1);
    }

    stream = gzdopen(fd, "r");

    while ((ct = gzread(stream, &buffer, 1024))) {
	write(1, &buffer, ct);
    }
    
    return 0;
}
