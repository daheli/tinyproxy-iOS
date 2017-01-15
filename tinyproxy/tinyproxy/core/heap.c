/* tinyproxy - A fast light-weight HTTP proxy
 * Copyright (C) 2002 Robert James Kaes <rjkaes@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/* Debugging versions of various heap related functions are combined
 * here.  The debugging versions include assertions and also print
 * (to standard error) the function called along with the amount
 * of memory allocated, and where the memory is pointing.  The
 * format of the log message is standardized.
 */

#include "main.h"
#include "heap.h"
#include "text.h"
#include "malloc.h"

#if ONLY_MSPACES
    mspace pool;
#endif

#ifndef NDEBUG

void *debugging_calloc (size_t nmemb, size_t size, const char *file,
                        unsigned long line)
{
        void *ptr;

        assert (nmemb > 0);
        assert (size > 0);

#if ONLY_MSPACES
        ptr = mspace_calloc(pool, nmemb, size);
#else
        ptr = calloc (nmemb, size);
#endif
        fprintf (stderr, "{calloc: %p:%lu x %lu} %s:%lu\n", ptr,
                 (unsigned long) nmemb, (unsigned long) size, file, line);
        return ptr;
}

void *debugging_malloc (size_t size, const char *file, unsigned long line)
{
        void *ptr;

        assert (size > 0);

#if ONLY_MSPACES
        ptr = mspace_malloc(pool, size);
#else
        ptr = malloc (size);
#endif
        fprintf (stderr, "{malloc: %p:%lu} %s:%lu\n", ptr,
                 (unsigned long) size, file, line);
        return ptr;
}

void *debugging_realloc (void *ptr, size_t size, const char *file,
                         unsigned long line)
{
        void *newptr;

        assert (size > 0);

#if ONLY_MSPACES
        newptr = mspace_realloc(pool, ptr, size);
#else
        newptr = realloc (ptr, size);
#endif
        fprintf (stderr, "{realloc: %p -> %p:%lu} %s:%lu\n", ptr, newptr,
                 (unsigned long) size, file, line);
        return newptr;
}

void debugging_free (void *ptr, const char *file, unsigned long line)
{
        fprintf (stderr, "{free: %p} %s:%lu\n", ptr, file, line);

        if (ptr != NULL)
#if ONLY_MSPACES
            mspace_free(pool, ptr);
#else
            free (ptr);
#endif
        return;
}

char *debugging_strdup (const char *s, const char *file, unsigned long line)
{
        char *ptr;
        size_t len;

        assert (s != NULL);

        len = strlen (s) + 1;
#if ONLY_MSPACES
        ptr = (char *) mspace_malloc (pool, len);
#else
        ptr = (char *) malloc (len);
#endif
        if (!ptr)
                return NULL;
        memcpy (ptr, s, len);

        fprintf (stderr, "{strdup: %p:%lu} %s:%lu\n", ptr,
                 (unsigned long) len, file, line);
        return ptr;
}

#endif /* !NDEBUG */

/*
 * Allocate a block of memory in the "shared" memory region.
 *
 * FIXME: This uses the most basic (and slowest) means of creating a
 * shared memory location.  It requires the use of a temporary file.  We might
 * want to look into something like MM (Shared Memory Library) for a better
 * solution.
 */
void *malloc_shared_memory (size_t size)
{
        int fd;
        void *ptr;
        char buffer[1024];

//        static const char *shared_file = "/tmp/tinyproxy.shared.XXXXXX";
    
        char new_buf[1024] = {0};
        snprintf(new_buf, sizeof(new_buf), "%s%s", TINYPROXY_BASE_DIR, "/tinyproxy.shared.XXXXXX");
        char *shared_file = &new_buf[0];
    
        assert (size > 0);

        strlcpy (buffer, shared_file, sizeof (buffer));

        /* Only allow u+rw bits. This may be required for some versions
         * of glibc so that mkstemp() doesn't make us vulnerable.
         */
        umask (0177);

        if ((fd = mkstemp (buffer)) == -1)
                return MAP_FAILED;
        unlink (buffer);

        if (ftruncate (fd, size) == -1)
                return MAP_FAILED;
        ptr = mmap (NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        return ptr;
}

/*
 * Allocate a block of memory from the "shared" region an initialize it to
 * zero.
 */
void *calloc_shared_memory (size_t nmemb, size_t size)
{
        void *ptr;
        long length;

        assert (nmemb > 0);
        assert (size > 0);

        length = nmemb * size;

        ptr = malloc_shared_memory (length);
        if (ptr == MAP_FAILED)
                return ptr;

        memset (ptr, 0, length);

        return ptr;
}

#if ONLY_MSPACES
int allocator_init(size_t max_size) {
    int fd;
    void *ptr;
    char unique_path[1024] = {0};
    snprintf(unique_path, 1024, "%s%s", TINYPROXY_BASE_DIR, "/tinyproxy.malloc.XXXXXX");
    
    if ((fd = mkstemp (unique_path)) == -1)
        return MAP_FAILED;
    unlink (unique_path);
    
    if (ftruncate (fd, max_size) == -1)
        return MAP_FAILED;
    ptr = mmap (NULL, max_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(ptr == MAP_FAILED) {
        fprintf (stderr, "map failed, err:%s", strerror(errno));
        return -1;
    }
    
    memset(ptr, 0, max_size);
    pool = create_mspace_with_base(ptr, max_size, 0);
    if(pool == NULL) {
        fprintf(stderr, "create pool failed, size:%d", max_size);
        return -1;
    }
    close(fd);
    
    return ptr == MAP_FAILED;;
}
#endif




