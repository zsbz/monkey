/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*  Monkey HTTP Daemon
 *  ------------------
 *  Copyright (C) 2008, Eduardo Silva P.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include "monkey.h"

#include "header.h"
#include "memory.h"
#include "iov.h"

struct mk_iov *mk_iov_create(int n, int offset)
{
	struct mk_iov *iov;

	iov = mk_mem_malloc(sizeof(struct mk_iov));
	iov->iov_idx = offset;
	iov->io = mk_mem_malloc(n*sizeof(struct iovec));
	iov->buf_to_free = mk_mem_malloc(n*sizeof(char *));
	iov->buf_idx = 0;
        iov->total_len = 0;
        iov->size = n;

	return iov;
}

int mk_iov_add_entry(struct mk_iov *mk_io, char *buf, int len,
                      mk_pointer sep, int free)
{
	mk_io->io[mk_io->iov_idx].iov_base = buf;
	mk_io->io[mk_io->iov_idx].iov_len = len;
	mk_io->iov_idx++;
        mk_io->total_len += len;

#ifdef DEBUG_IOV
        if(mk_io->iov_idx > mk_io->size){
                printf("\nDEBUG IOV :: ERROR, Broke array size in:");
                printf("\n          '''%s'''", buf); 
                fflush(stdout);
        }
#endif

        /* Add separator */
        if(sep.len > 0)
        {
                mk_io->io[mk_io->iov_idx].iov_base = sep.data;
                mk_io->io[mk_io->iov_idx].iov_len = sep.len;
                mk_io->iov_idx++; 
                mk_io->total_len += sep.len;
        }
        
        if(free == MK_IOV_FREE_BUF)
        {
                _mk_iov_set_free(mk_io, buf);
        }

	return mk_io->iov_idx;
}

int mk_iov_set_entry(struct mk_iov *mk_io, char *buf, int len, 
                     int free, int idx)
{
       	mk_io->io[idx].iov_base = buf;
	mk_io->io[idx].iov_len = len;
        mk_io->total_len += len;

        if(free == MK_IOV_FREE_BUF)
        {
                _mk_iov_set_free(mk_io, buf);
        }

        return 0;
}

void _mk_iov_set_free(struct mk_iov *mk_io, char *buf)
{
	mk_io->buf_to_free[mk_io->buf_idx] = (char *)buf;
	mk_io->buf_idx++;
}

ssize_t mk_iov_send(int fd, struct mk_iov *mk_io)
{
	ssize_t n;

	n = writev(fd, mk_io->io, mk_io->iov_idx);
	return n;
}

void mk_iov_free(struct mk_iov *mk_io)
{
        mk_iov_free_marked(mk_io);
	mk_mem_free(mk_io->io);
	mk_mem_free(mk_io);
}
void mk_iov_free_marked(struct mk_iov *mk_io)
{
	int i, limit=0;

	limit = mk_io->buf_idx;

	for(i=0; i<limit; i++)
	{

#ifdef DEBUG_IOV
                printf("\nDEBUG IOV :: going free (idx: %i/%i): %s",i,
                       limit, mk_io->buf_to_free[i]);
                fflush(stdout);
#endif
		mk_mem_free(mk_io->buf_to_free[i]);
	}

        mk_io->iov_idx = 0;
        mk_io->buf_idx = 0;
}

void mk_iov_print(struct mk_iov *mk_io)
{
        int i;

        for(i=0; i<mk_io->iov_idx; i++){
                printf("\n%i len=%i) '%s'", i, mk_io->io[i].iov_len, 
                       (char *) mk_io->io[i].iov_base);
                fflush(stdout);
        }
}

void mk_iov_separators_init()
{
        mk_pointer_set(&mk_iov_crlf, MK_IOV_CRLF);
        mk_pointer_set(&mk_iov_lf, MK_IOV_LF);
        mk_pointer_set(&mk_iov_space, MK_IOV_SPACE);
        mk_pointer_set(&mk_iov_header_value, MK_IOV_HEADER_VALUE);
        mk_pointer_set(&mk_iov_slash, MK_IOV_SLASH);
        mk_pointer_set(&mk_iov_none, MK_IOV_NONE);
        mk_pointer_set(&mk_iov_equal, MK_IOV_EQUAL);
}
