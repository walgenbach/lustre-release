/* -*- mode: c; c-basic-offset: 8; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 *  lustre/mds/mds_audit.c
 *  Lustre Metadata Server (mds) audit stuff
 *
 *  Copyright (c) 2001-2003 Cluster File Systems, Inc.
 *
 *   This file is part of Lustre, http://www.lustre.org.
 *
 *   Lustre is free software; you can redistribute it and/or
 *   modify it under the terms of version 2 of the GNU General Public
 *   License as published by the Free Software Foundation.
 *
 *   Lustre is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Lustre; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef EXPORT_SYMTAB
# define EXPORT_SYMTAB
#endif
#define DEBUG_SUBSYSTEM S_MDS

#include <linux/module.h>
#include <linux/lustre_mds.h>
#include <linux/lustre_dlm.h>
#include <linux/init.h>
#include <linux/obd_class.h>
#include <linux/random.h>
#include <linux/fs.h>
#include <linux/jbd.h>
#include <linux/namei.h>
#include <linux/ext3_fs.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0))
# include <linux/smp_lock.h>
# include <linux/buffer_head.h>
# include <linux/workqueue.h>
# include <linux/mount.h>
#else
# include <linux/locks.h>
#endif
#include <linux/obd_lov.h>
#include <linux/obd_ost.h>
#include <linux/lustre_mds.h>
#include <linux/lustre_fsfilt.h>
#include <linux/lprocfs_status.h>
#include <linux/lustre_commit_confd.h>
#include <linux/lustre_acl.h>
#include <linux/lustre_audit.h>
#include "mds_internal.h"

int mds_audit_stat(struct ptlrpc_request *req, struct lustre_id * id,
                   struct dentry * dentry, int ret)
{
        struct obd_device *obd = req->rq_export->exp_obd;
        struct inode * inode = dentry->d_inode;
        ptl_nid_t nid = req->rq_peer.peer_id.nid;
        struct audit_info info;
        int rc = 0, len = sizeof(info);
        ENTRY;
        
        LASSERT(inode);
        LASSERT(id);
        memcpy(&info.m.id, id, sizeof(*id));
        info.m.nid = nid;
        info.m.uid = current->uid;
        info.m.gid = current->gid;
        info.m.result = ret;
        info.m.code = AUDIT_STAT;
   
        //send info to local fs
        fsfilt_set_info(obd, inode->i_sb, inode,
                        10, "audit_info", len, (void*)&info);

        RETURN(rc);
}

int mds_audit_open(struct ptlrpc_request *req, struct mds_update_record * rec,
                   int ret)
{
        struct obd_device *obd = req->rq_export->exp_obd;
        struct inode * inode = NULL;
        ptl_nid_t nid = req->rq_peer.peer_id.nid;
        struct audit_info info;
        int rc = 0, len = sizeof(info);
        
        struct dentry * dparent = NULL;
        
        dparent = mds_id2dentry(obd, rec->ur_id1, NULL);
        if (IS_ERR(dparent)) {
                rc = PTR_ERR(dparent);
                RETURN(rc);
        }
        inode = dparent->d_inode;
        
        info.m.id = *(rec->ur_id1);
        info.m.nid = nid;
        info.m.uid = rec->ur_uc.luc_uid;
        info.m.gid = rec->ur_uc.luc_gid;
        info.m.result = ret;
        info.m.code = AUDIT_OPEN;
        info.name = rec->ur_name;
        info.namelen = rec->ur_namelen;
        
        //send info to local fs
        fsfilt_set_info(obd, inode->i_sb, inode,
                        10, "audit_info", len, (void*)&info);

        l_dput(dparent);

        RETURN(rc);
}

int mds_audit_auth(struct ptlrpc_request *req, struct lvfs_ucred * uc,
                   audit_op op, struct lustre_id * id,
                   char * name, int namelen)
{
        struct obd_device *obd = req->rq_export->exp_obd;
        ptl_nid_t nid = req->rq_peer.peer_id.nid;
        int rc = 0;
        struct dentry * dparent;
        struct inode * inode;
        struct audit_info info;
        
        ENTRY;
        
        dparent = mds_id2dentry(obd, id, NULL);
        if (IS_ERR(dparent)) {
                rc = PTR_ERR(dparent);
                GOTO(out, rc);
        }
        inode = dparent->d_inode;
        
        info.m.nid = nid;
        info.m.uid = uc->luc_uid;
        info.m.gid = uc->luc_gid;
        info.m.id = (*id);
        info.m.result = -EPERM;
        info.m.code = op;
        info.name = name;
        info.namelen = namelen;
        
        fsfilt_set_info(obd, inode->i_sb, inode,
                        10, "audit_info", sizeof(info), &info);
        l_dput(dparent);

        EXIT;
 out:
        return rc;
}

static int mds_set_obj_audit(struct obd_device * obd, struct inode * inode,
                      __u64 * mask)
{
        struct audit_lov_msg msg = {
                .mask = *mask,
                .lsm = NULL,
                .uid = inode->i_uid,
                .gid = inode->i_gid,
        };
        int len, rc = 0;
        void *lmm = NULL;
        
        down(&inode->i_sem);
        len = fsfilt_get_md(obd, inode, NULL, 0, EA_LOV);
        up(&inode->i_sem);
        
        if (len < 0) {
                CERROR("error getting inode %lu LOV: %d\n", inode->i_ino, len);
                GOTO(out, rc = len);
        } else if (len == 0) {
                CDEBUG(D_INODE, "no LOV in inode %lu\n", inode->i_ino);
                GOTO(out, rc = 0);
        }
        
        OBD_ALLOC(lmm, len);
        if (lmm == NULL) {
                CERROR("can't allocate memory\n");
                GOTO(out, rc = -ENOMEM);
        }

        down(&inode->i_sem);
        rc = fsfilt_get_md(obd, inode, lmm, len, EA_LOV);
        up(&inode->i_sem);
        
        if (rc < 0) {
                CERROR("error getting inode %lu MD: %d\n", inode->i_ino, rc);
                GOTO(out, rc);
        }

        rc = obd_unpackmd(obd->u.mds.mds_dt_exp, &msg.lsm, lmm, len);
        if (rc < 0) {
                CERROR("error unpacking inode %lu MD: %d\n", inode->i_ino, rc);
                GOTO(out, rc);
        }

        obd_set_info(obd->u.mds.mds_dt_exp, 9, "audit_obj", sizeof(msg), &msg);
        
out:   
        if (msg.lsm != NULL)
                obd_free_memmd(obd->u.mds.mds_dt_exp, &msg.lsm);
        
        if (lmm != NULL)
                OBD_FREE(lmm, len);

        RETURN(rc);
}

//set audit attributes for directory/file
int mds_set_audit(struct obd_device * obd, void * val)
{
        struct inode * inode = NULL;
        struct dentry * dentry = NULL;
        //struct lvfs_run_ctxt saved;
        struct audit_attr_msg * msg = val;
   
        ENTRY;
        
        //push_ctxt(&saved, &obd->obd_lvfs_ctxt, NULL);

        dentry = mds_id2dentry(obd, &msg->id, NULL);
        if (IS_ERR(dentry)) {
                CERROR("Cannot get dentry\n");
                RETURN(PTR_ERR(dentry));
        }
            
        inode = dentry->d_inode;
        fsfilt_set_info(obd, inode->i_sb, inode,
                        5, "audit", sizeof(msg->attr), &msg->attr);
        
        if (S_ISREG(inode->i_mode) && !IS_AUDIT_OP(msg->attr, AUDIT_FS))
                mds_set_obj_audit(obd, inode, &msg->attr);
        
        l_dput(dentry);
        
        //pop_ctxt(&saved, &obd->obd_lvfs_ctxt, NULL);

        RETURN(0);                                
}

int mds_pack_audit(struct obd_device * obd, struct inode * inode,
                  struct mds_body * body)
{
        __u64 mask = 0;
        int len = sizeof(mask);
        int rc = 0;
        
        rc = fsfilt_get_info(obd, inode->i_sb, inode, 
                             5, "audit", &len, &mask);
        if (!rc) {
                body->audit = mask;
                body->valid |= OBD_MD_FLAUDIT;
        }        
        return rc;
}
