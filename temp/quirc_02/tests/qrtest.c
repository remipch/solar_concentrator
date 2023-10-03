/* quirc -- QR-code recognition library
 * Copyright (C) 2010-2012 Daniel Beer <dlbeer@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <quirc.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <time.h>
#include "dbgutil.h"

static struct quirc *decoder;

static int scan_file(const char *path, const char *filename)
{
	int (*loader)(struct quirc *, const char *);
	int len = strlen(filename);
	const char *ext;
	int ret;
	int i;

	while (len >= 0 && filename[len] != '.')
		len--;
	ext = filename + len + 1;
	if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0)
		loader = load_jpeg;
	else if (strcasecmp(ext, "png") == 0)
		loader = load_png;
	else
		return 0;

    printf("Scan file %s :\n", filename);

	ret = loader(decoder, path);

	if (ret < 0) {
		fprintf(stderr, "%s: load failed\n", filename);
		return -1;
	}

	quirc_end(decoder);
    dump_capstones(decoder);

    return 1;
}

static int test_scan(const char *path);

static int scan_dir(const char *path, const char *filename)
{
	DIR *d = opendir(path);
	struct dirent *ent;
	int count = 0;

	if (!d) {
		fprintf(stderr, "%s: opendir: %s\n", path, strerror(errno));
		return -1;
	}

	printf("%s:\n", path);

	while ((ent = readdir(d))) {
		if (ent->d_name[0] != '.') {
			char fullpath[1024];

			snprintf(fullpath, sizeof(fullpath), "%s/%s",
				 path, ent->d_name);
			if (test_scan(fullpath) > 0) {
				count++;
			}
		}
	}

	closedir(d);

	return count > 0;
}

static int test_scan(const char *path)
{
	int len = strlen(path);
	struct stat st;
	const char *filename;

	while (len >= 0 && path[len] != '/')
		len--;
	filename = path + len + 1;

	if (lstat(path, &st) < 0) {
		fprintf(stderr, "%s: lstat: %s\n", path, strerror(errno));
		return -1;
	}

	if (S_ISREG(st.st_mode))
		return scan_file(path, filename);

	if (S_ISDIR(st.st_mode))
		return scan_dir(path, filename);

	return 0;
}

static int run_tests(int argc, char **argv)
{
	int count = 0;
	int i;

	decoder = quirc_new();
	if (!decoder) {
		perror("quirc_new");
		return -1;
	}

	for (i = 0; i < argc; i++) {
		if (test_scan(argv[i]) > 0) {
			count++;
		}
	}

	if (count > 1)
		printf("TOTAL: %i", count);

	quirc_destroy(decoder);
	return 0;
}

int main(int argc, char **argv)
{
	return run_tests(argc, argv);;
}
