/*
 * Copryright (C) 2001 Cluster File Systems, Inc.
 *
 */

#define EXPORT_SYMTAB

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/errno.h>
#include <linux/locks.h>
#include <linux/unistd.h>

#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/stat.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <asm/segment.h>
#include <linux/miscdevice.h>

#include <linux/obd_support.h>
#include <linux/lustre_lib.h>
#include <linux/lustre_idl.h>
#include <linux/lustre_mds.h>

extern int mdc_reint(struct mds_request *request);
extern struct mds_request *mds_prep_req(int opcode, int namelen, char *name, int tgtlen, char *tgt);

int mdc_setattr(struct inode *inode, struct iattr *iattr,
		struct mds_rep **rep, struct mds_rep_hdr **hdr)
{
	int rc; 
	struct mds_request *request;
	struct mds_rec_setattr *rec;

	request = mds_prep_req(MDS_REINT, 0, NULL, sizeof(*rec), NULL);
	if (!request) { 
		printk("mdc request: cannot pack\n");
		return -ENOMEM;
	}

	rec = mds_req_tgt(request->rq_req);
	mds_setattr_pack(rec, inode, iattr); 
	request->rq_req->opcode = HTON__u32(REINT_SETATTR);

	rc = mdc_reint(request);

	if (rep) { 
		*rep = request->rq_rep;
	}
	if (hdr) { 
		*hdr = request->rq_rephdr;
	}

	kfree(request); 
	return rc;
}

int mdc_create(struct inode *dir, const char *name, int namelen, 
	       int mode, __u64 id, __u32 uid, __u32 gid, __u64 time, 
		struct mds_rep **rep, struct mds_rep_hdr **hdr)
{
	int rc; 
	struct mds_request *request;
	struct mds_rec_create *rec;

	request = mds_prep_req(MDS_REINT, 0, NULL, 
			       sizeof(*rec) + size_round(namelen + 1), 
			       NULL);
	if (!request) { 
		printk("mdc_create: cannot pack\n");
		return -ENOMEM;
	}

	rec = mds_req_tgt(request->rq_req);
	mds_create_pack(rec, dir, name, namelen, mode, id, uid, gid, time); 

	rc = mdc_reint(request);

	if (rep) { 
		*rep = request->rq_rep;
	}
	if (hdr) { 
		*hdr = request->rq_rephdr;
	}

	kfree(request); 
	return rc;
}
