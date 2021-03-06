/*
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; If not, see
 * http://www.sun.com/software/products/lustre/docs/GPLv2.pdf
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *
 * GPL HEADER END
 */
/*
 * Copyright (c) 2007, 2010, Oracle and/or its affiliates. All rights reserved.
 * Use is subject to license terms.
 */
/*
 * This file is part of Lustre, http://www.lustre.org/
 * Lustre is a trademark of Sun Microsystems, Inc.
 *
 * lustre/include/lvfs.h
 *
 * lustre VFS/process permission interface
 */

#ifndef __LVFS_H__
#define __LVFS_H__

#define LL_FID_NAMELEN (16 + 1 + 8 + 1)

#include <libcfs/libcfs.h>
#if defined(__linux__)
#include <linux/lvfs.h>
#elif defined(__APPLE__)
#include <darwin/lvfs.h>
#elif defined(__WINNT__)
#include <winnt/lvfs.h>
#else
#error Unsupported operating system.
#endif

#include <libcfs/lucache.h>

#ifdef LIBLUSTRE
#include <lvfs_user_fs.h>
#endif

/* lvfs_linux.c */
void push_ctxt(struct lvfs_run_ctxt *save, struct lvfs_run_ctxt *new_ctx,
               struct lvfs_ucred *cred);
void pop_ctxt(struct lvfs_run_ctxt *saved, struct lvfs_run_ctxt *new_ctx,
              struct lvfs_ucred *cred);
#endif
