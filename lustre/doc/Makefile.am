#
# GPL HEADER START
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 only,
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License version 2 for more details (a copy is included
# in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU General Public License
# version 2 along with this program; If not, see
# http://www.sun.com/software/products/lustre/docs/GPLv2.pdf
#
# Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
# CA 95054 USA or visit www.sun.com if you need additional information or
# have any questions.
#
# GPL HEADER END
#

#
# Copyright (c) 2001, 2010, Oracle and/or its affiliates. All rights reserved.
# Use is subject to license terms.
#
# Copyright (c) 2011, Intel Corporation.
#

#
# This file is part of Lustre, http://www.lustre.org/
# Lustre is a trademark of Sun Microsystems, Inc.
#

MANFILES = lustre.7 lfs.1 mount.lustre.8 lctl.8 \
	llverdev.8 llbackup.8 llapi_quotactl.3 llobdstat.8 llstat.8 \
	plot-llstat.8 l_getgroups.8 lst.8 routerstat.8 lshowmount.8 \
	ll_recover_lost_found_objs.8 llog_reader.8 llapi_file_open.3 \
	llapi_file_create.3 llapi_file_get_stripe.3 lustreapi.7 \
	lustre_rsync.8 lfs_migrate.1 lhbadm.8 ldev.8 ldev.conf.5 nids.5 \
	lfs-hsm.1 llapi_hsm_state_get.3 llapi_hsm_state_set.3 \
	lustre_routes_config.8 lustre_routes_conversion.8
SERVER_MANFILES = mkfs.lustre.8 tunefs.lustre.8

if SERVER
MANFILES += $(SERVER_MANFILES)
else
DISTONLY_MANFILES = $(SERVER_MANFILES)
endif

if UTILS
man_MANS = $(MANFILES)
endif

CLEANFILES = *.aux *.tex *.log *.pdf

EXTRA_DIST = $(MANFILES) $(DISTONLY_MANFILES)

all:
