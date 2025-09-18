/*
 * Copyright (c) 2025 Michael Clark <michaeljclark@mac.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
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
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

#ifdef _WIN32
#include <windows.h>
#define PATH_MAX 1024
#elif __APPLE__
#include <mach-o/dyld.h>
#elif __linux__
#include <unistd.h>
#include <sys/stat.h>
#elif __FreeBSD__
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#include "lv_osutil.h"

/*
 * get zero-terminated executable path possibly with truncation
 *
 * @param [out] path name buffer
 * @param [in] path name buffer size
 *
 * @return 0 for success, -1 for error
 */
int get_executable_path(char *buf, size_t buflen)
{
    if (buflen < 1) {
        return -1;
    }
#ifdef _WIN32
    /* GetModuleFileNameA is not zero-terminated if truncated,
     * reserve space and unconditionally zero terminate */
    DWORD size = GetModuleFileNameA(NULL, buf, (DWORD)buflen - 1);
    if (size != 0) {
        buf[size] = '\0';
        return 0;
    }
#elif __APPLE__
    /* if truncated _NSGetExecutablePath will zero-terminate for us */
    uint32_t size  = buflen;
    int ret = _NSGetExecutablePath(buf, &size);
    if (ret == 0) {
        return 0;
    }
#elif __linux__
    /* readlink is not zero-terminated and may be truncated,
     * reserve space and unconditionally zero terminate */
    ssize_t size = readlink("/proc/self/exe", buf, buflen - 1);
    if (size != -1) {
        buf[size] = '\0';
        return 0;
    }
#elif __FreeBSD__
    int mib[4] = {
        CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 /* current process */
    };
    /* sysctl is not zero-terminated if truncated,
     * reserve space and unconditionally zero terminate */
    size_t size = buflen - 1;
    int ret = sysctl(mib, 4, buf, &size, NULL, 0);
    if (ret == 0) {
        buf[size] = '\0';
        return 0;
    }
#endif
    buf[0] = '\0';
    return -1;
}

/*
 * get zero-terminated executable directory possibly with truncation
 *
 * @param [out] directory name buffer
 * @param [in] directory name buffer size
 *
 * @return 0 for success, -1 for error
 */
int get_executable_dir(char *buf, size_t buflen)
{
    /* this ensures there is room for zero-termination */
    if (buflen < 2 || get_executable_path(buf, buflen) != 0) {
        return -1;
    }

    char *last_sep = strrchr(buf, '/');

#ifdef _WIN32
    char *last_bslash = strrchr(buf, '\\');
    if (last_bslash > last_sep) {
        last_sep = last_bslash;
    }
#endif

    if (last_sep) {
        *last_sep = '\0';
        return 0;
    }
    buf[0] = '.';
    buf[1] = '\0';
    return -1;
}

/*
 * get zero-terminated resource path possibly with truncation
 *
 * @param [out] path name buffer
 * @param [in] path name buffer size
 * @param [in] resource resrouce name buffer
 *
 * @return 0 for success, -1 for error
 */
int get_resource_path(char *buf, size_t buflen, const char *resource)
{
    char tmp[PATH_MAX];

    if (buflen < 1) {
        return -1;
    }

    int ret = get_executable_dir(tmp, sizeof(tmp));
    if (ret != 0) {
        return -1;
    }

    size_t len = snprintf(buf, buflen, "%s%c%s", tmp, PATH_SEPARATOR, resource);
    (void)len;

    return 0;
}
