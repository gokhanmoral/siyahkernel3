/*
 * j4fs_fs.c
 *
 * External interface to other kernel subsystems
 *
 * COPYRIGHT(C) Samsung Electronics Co.Ltd. 2009-2010 All Right Reserved.
 *
 * 2009.02 - First editing by SungHwan.yun <sunghwan.yun@samsung.com> @LDK@
 *
 * 2009.03 - Currently managed by  SungHwan.yun <sunghwan.yun@samsung.com> @LDK@
 *
 */
#include <linux/time.h>
#include <linux/highuid.h>
#include <linux/pagemap.h>
#include <linux/quotaops.h>
#include <linux/module.h>
#include <linux/writeback.h>
#include <linux/buffer_head.h>
#include <linux/mpage.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include "j4fs.h"

#if defined(J4FS_USE_XSR)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28))
#include "./Inc/XsrTypes.h"
#include "./Inc/STL.h"
#else
#include "../../drivers/txsr/Inc/XsrTypes.h"
#include "../../drivers/txsr/Inc/STL.h"
#endif

#elif defined(J4FS_USE_FSR)

#include "../../fsr/Inc/FSR.h"
#include "../../fsr/Inc/FSR_STL.h"

// J4FS for moviNAND merged from ROSSI
#elif defined(J4FS_USE_MOVI)
// J4FS for moviNAND merged from ROSSI
#else
'compile error'
#endif


#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 27))
#define J4FS_USE_WRITE_BEGIN_END 1
#else
#define J4FS_USE_WRITE_BEGIN_END 0
#endif

#define Page_Uptodate(page)	test_bit(PG_uptodate, &(page)->flags)

extern j4fs_device_info device_info;
extern unsigned int j4fs_traceMask;
extern unsigned int j4fs_rw_start;
extern int ro_j4fs_header_count;
extern unsigned int j4fs_next_sequence;
extern unsigned int j4fs_transaction_next_offset;
extern int j4fs_panic;

void j4fs_GrossLock(void)
{
	J4FS_T(J4FS_TRACE_LOCK, ("j4fs locking %p\n", current));
	down(&device_info.grossLock);
	J4FS_T(J4FS_TRACE_LOCK, ("j4fs locked %p\n", current));
}

void j4fs_GrossUnlock(void)
{
	J4FS_T(J4FS_TRACE_LOCK, ("j4fs unlocking %p\n", current));
	up(&device_info.grossLock);
}

int j4fs_readpage(struct file *f, struct page *page)
{
	J4FS_T(J4FS_TRACE_FS_READ,("%s %d\n",__FUNCTION__,__LINE__));
	return j4fs_readpage_unlock(f, page);
}

int j4fs_readpage_unlock(struct file *f, struct page *page)
{
	int ret = j4fs_readpage_nolock(f, page);
	unlock_page(page);
	return ret;
}

int j4fs_readpage_nolock(struct file *f, struct page *page)
{
	/* Lifted from yaffs2 */
	unsigned char *page_buf;
	int ret;
	struct address_space *mapping = page->mapping;
	struct inode *inode;
	j4fs_ctrl ctl;

	J4FS_T(J4FS_TRACE_FS_READ,("%s %d\n",__FUNCTION__,__LINE__));

	BUG_ON(!PageLocked(page));

	if (!mapping) BUG();

	inode = mapping->host;

	if (!inode) BUG();

	page_buf = kmap(page);
	/* FIXME: Can kmap fail? */

	j4fs_GrossLock();

	ctl.buffer=page_buf;
	ctl.count=PAGE_CACHE_SIZE;
	ctl.id=inode->i_ino;
	ctl.index=page->index << PAGE_CACHE_SHIFT;
	ret=fsd_read(&ctl);

	j4fs_GrossUnlock();

	if (ret >= 0)
		ret = 0;

	if (ret) {
		ClearPageUptodate(page);
		SetPageError(page);
	} else {
		SetPageUptodate(page);
		ClearPageError(page);
	}

	flush_dcache_page(page);
	kunmap(page);

	return ret;
}

int j4fs_writepage(struct page *page, struct writeback_control *wbc)
{
	struct address_space *mapping = page->mapping;
	loff_t offset = (loff_t) page->index << PAGE_CACHE_SHIFT;
	struct inode *inode;
	unsigned long end_index;
	char *buffer;
	int nWritten = 0;
	unsigned nBytes;
	j4fs_ctrl ctl;
	int nErr;

	if(j4fs_panic==1) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: j4fs panic\n",__FUNCTION__,__LINE__));
		return -ENOSPC;
	}

	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));

	if (!mapping) BUG();

	inode = mapping->host;

	if (!inode) BUG();

	if (offset > inode->i_size) {
		J4FS_T(J4FS_TRACE_FS,
			("j4fs_writepage at %08x, inode size = %08x!!!\n",
			(unsigned)(page->index << PAGE_CACHE_SHIFT),
			(unsigned)inode->i_size));
		J4FS_T(J4FS_TRACE_FS,
			("                -> don't care!!\n"));
		unlock_page(page);
		return 0;
	}

	end_index = inode->i_size >> PAGE_CACHE_SHIFT;

	/* easy case */
	if (page->index < end_index)
		nBytes = PAGE_CACHE_SIZE;
	else
		nBytes = inode->i_size & (PAGE_CACHE_SIZE - 1);

	get_page(page);

	buffer = kmap(page);

	j4fs_GrossLock();

	J4FS_T(J4FS_TRACE_FS,
		("j4fs_writepage: index=%08x,nBytes=%08x,inode.i_size=%05x\n", (unsigned)(page->index << PAGE_CACHE_SHIFT), nBytes,(int)inode->i_size));

	// write file
	ctl.buffer=buffer;
	ctl.count=nBytes;
	ctl.id=inode->i_ino;
	ctl.index=offset;

	nErr=fsd_write(&ctl);

	if(nErr==J4FS_RETRY_WRITE) nErr=fsd_write(&ctl);

	J4FS_T(J4FS_TRACE_FS,
		("j4fs_writepage: index=%08x,nBytes=%08x,inode.i_size=%05x\n", (unsigned)(page->index << PAGE_CACHE_SHIFT), nBytes,(int)inode->i_size));

	j4fs_GrossUnlock();

	kunmap(page);
	SetPageUptodate(page);
	unlock_page(page);
	put_page(page);

	return (nWritten == nBytes) ? 0 : -ENOSPC;

}

#if (J4FS_USE_WRITE_BEGIN_END > 0)
int j4fs_write_begin(struct file *filp, struct address_space *mapping,
				loff_t pos, unsigned len, unsigned flags,
				struct page **pagep, void **fsdata)
{
	struct page *pg = NULL;
	pgoff_t index = pos >> PAGE_CACHE_SHIFT;
	uint32_t offset = pos & (PAGE_CACHE_SIZE - 1);
	uint32_t to = offset + len;

	int ret = 0;
	int space_held = 0;

	if(j4fs_panic==1) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: j4fs panic\n",__FUNCTION__,__LINE__));
		return -ENOSPC;
	}

	J4FS_T(J4FS_TRACE_FS, ("start j4fs_write_begin\n"));

	if(to>PAGE_CACHE_SIZE) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: page size overflow(pos,index,offset,len,to)=(%lld,%lu,%u,%d,%d)\n",__FUNCTION__,__LINE__,pos,index,offset,len,to));
		j4fs_panic("page size overflow");
		return -ENOSPC;
	}

	/* Get a page */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
	pg = grab_cache_page_write_begin(mapping, index, flags);
#else
	pg = __grab_cache_page(mapping, index);
#endif

	*pagep = pg;
	if (!pg) {
		ret =  -ENOMEM;
		goto out;
	}

	/* Get fs space */
	space_held = j4fs_hold_space(PAGE_CACHE_SIZE);

	if (!space_held) {
		ret = -ENOSPC;
		goto out;
	}

	/* Update page if required */
	if (!Page_Uptodate(pg) && (offset || to < PAGE_CACHE_SIZE))
		ret = j4fs_readpage_nolock(filp, pg);

	if (ret)
		goto out;

	/* Happy path return */
	J4FS_T(J4FS_TRACE_FS, ("end j4fs_write_begin - ok\n"));

	return 0;

out:
	J4FS_T(J4FS_TRACE_FS, ("end j4fs_write_begin fail returning %d\n", ret));

	if (pg) {
		unlock_page(pg);
		page_cache_release(pg);
	}
	return ret;
}

#else

int j4fs_prepare_write(struct file *f, struct page *pg,
				unsigned offset, unsigned to)
{
	if(j4fs_panic==1) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: j4fs panic\n",__FUNCTION__,__LINE__));
		return -ENOSPC;
	}

	J4FS_T(J4FS_TRACE_FS, ("\nj4fs_prepare_write\n"));

	if(to>PAGE_CACHE_SIZE) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: page size overflow(offset,to)=(%d,%d)\n",__FUNCTION__,__LINE__,offset,to));
		j4fs_panic("page size overflow");
		return -ENOSPC;
	}

	if (!Page_Uptodate(pg) && (offset || to < PAGE_CACHE_SIZE))
		return j4fs_readpage_nolock(f, pg);
	return 0;
}
#endif

#if (J4FS_USE_WRITE_BEGIN_END > 0)
int j4fs_write_end(struct file *filp, struct address_space *mapping,
				loff_t pos, unsigned len, unsigned copied,
				struct page *pg, void *fsdadata)
{
	int ret = 0;
	void *addr, *kva;
	uint32_t offset_into_page = pos & (PAGE_CACHE_SIZE - 1);

	if(j4fs_panic==1) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: j4fs panic\n",__FUNCTION__,__LINE__));
		return -ENOSPC;
	}

	if(offset_into_page+copied > PAGE_CACHE_SIZE) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: page size overflow(offset_into_page,copied)=(%d,%d)\n",__FUNCTION__,__LINE__,offset_into_page, copied));
		j4fs_panic("page size overflow");
		return -ENOSPC;
	}

	kva = kmap(pg);
	addr = kva + offset_into_page;

	J4FS_T(J4FS_TRACE_FS,
		("j4fs_write_end addr %x pos %x nBytes %d\n",
		(unsigned) addr,
		(int)pos, copied));

	ret = j4fs_file_write(filp, addr, copied, &pos);

	if (ret != copied) {
		J4FS_T(J4FS_TRACE_ALWAYS, ("j4fs_write_end not same size ret %d  copied %d\n", ret, copied));
		SetPageError(pg);
		ClearPageUptodate(pg);
	} else {
		SetPageUptodate(pg);
	}

	kunmap(pg);

	unlock_page(pg);
	page_cache_release(pg);
	return ret;
}
#else

int j4fs_commit_write(struct file *f, struct page *pg, unsigned offset, unsigned to)
{
	void *addr, *kva;

	loff_t pos = (((loff_t) pg->index) << PAGE_CACHE_SHIFT) + offset;
	int nBytes = to - offset;
	int nWritten;

	unsigned spos = pos;
	unsigned saddr;

	if(j4fs_panic==1) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: j4fs panic\n",__FUNCTION__,__LINE__));
		return -ENOSPC;
	}

	if(offset+nBytes > PAGE_CACHE_SIZE) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: page size overflow(offset,nBytes)=(%d,%d)\n",__FUNCTION__,__LINE__, offset, nBytes));
		j4fs_panic("page size overflow");
		return -ENOSPC;
	}

	kva = kmap(pg);
	addr = kva + offset;

	saddr = (unsigned) addr;

	J4FS_T(J4FS_TRACE_FS, ("j4fs_commit_write: (addr,pos,nBytes)=(0x%x, 0x%x, 0x%x)\n", saddr, spos, nBytes));

	nWritten = j4fs_file_write(f, addr, nBytes, &pos);

	if (nWritten != nBytes) {
		J4FS_T(J4FS_TRACE_ALWAYS, ("j4fs_commit_write: (nWritten,nBytes)=(0x%x 0x%x)\n", nWritten, nBytes));
		SetPageError(pg);
		ClearPageUptodate(pg);
	} else {
		SetPageUptodate(pg);
	}

	kunmap(pg);

	return nWritten == nBytes ? 0 : nWritten;
}
#endif

int j4fs_file_write(struct file *f, const char *buf, size_t n, loff_t *pos)
{
	int nWritten, ipos;
	struct inode *inode;
	j4fs_ctrl ctl;

	if(j4fs_panic==1) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: j4fs panic\n",__FUNCTION__,__LINE__));
		return -ENOSPC;
	}

	j4fs_GrossLock();

	inode = f->f_dentry->d_inode;

	if (!S_ISBLK(inode->i_mode) && f->f_flags & O_APPEND)
		ipos = inode->i_size;
	else
		ipos = *pos;

	J4FS_T(J4FS_TRACE_FS,("j4fs_file_write: %zu bytes to ino %ld at %d\n", n, inode->i_ino, ipos));

	// write file
	ctl.buffer=(BYTE *)buf;
	ctl.count=n;
	ctl.id=inode->i_ino;
	ctl.index=ipos;

	nWritten=fsd_write(&ctl);

	if(nWritten==J4FS_RETRY_WRITE) nWritten=fsd_write(&ctl);

	if(nWritten==J4FS_RETRY_WRITE || error(nWritten))
	{
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nWritten=0x%x)\n",__FUNCTION__,__LINE__,nWritten));
		j4fs_GrossUnlock();
		return -ENOSPC;
	}

	if (nWritten > 0) {
		ipos += nWritten;
		*pos = ipos;
		if (ipos > inode->i_size) {
			inode->i_size = ipos;
			inode->i_blocks = (ipos + 511) >> 9;
		}

	}
	j4fs_GrossUnlock();
	return nWritten == 0 ? -ENOSPC : nWritten;
}

struct j4fs_inode *j4fs_get_inode(struct super_block *sb, ino_t ino)
{
	unsigned int cur_link, latest_matching_offset=0xffffffff;
	struct j4fs_inode *raw_inode;
	int nErr;
	BYTE *buf;

	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));

	buf=kmalloc(J4FS_BASIC_UNIT_SIZE,GFP_NOFS);

	if(j4fs_panic==1) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: j4fs panic\n",__FUNCTION__,__LINE__));
		goto error1;
	}

	if (ino != J4FS_ROOT_INO && ino < J4FS_FIRST_INO) goto Einval;

	if(ino==J4FS_ROOT_INO) goto error1;

	// read j4fs_header in flash which inode number is ino
	cur_link=device_info.j4fs_offset;
	while(cur_link!=0xffffffff)
	{
		// check the partition range
		j4fs_check_partition_range(cur_link);

		nErr = FlashDevRead(&device_info, cur_link, J4FS_BASIC_UNIT_SIZE, buf);
		if (nErr != 0) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: error(nErr=0x%x)\n",__FUNCTION__,__LINE__,nErr));
	   		goto error1;
		}

		raw_inode = (struct j4fs_inode *)buf;

		//This j4fs_header cannot be interpreted. It means there are no files in this partition(this can happen and this is a normal case) or
		//this j4fs partition is crashed(this should not happen).
		if(raw_inode->i_type!=J4FS_FILE_TYPE)
		{
			// There are no files in this partition or this first j4fs_header is crashed. So, this case should not happen and/or should be repaired.
			if(cur_link==device_info.j4fs_offset) {
				j4fs_panic("There are no files in this partition or this first j4fs_header is crashed. So, this case should not happen and/or should be repaired.");
				goto error1;
			}

			// This j4fs partition is crashed by some abnormal cause. This should not happen and should be repaired.
			j4fs_panic("this j4fs partition is crashed by some abnormal cause.  This should not happen and should be repaired.");
			goto error1;
		}

		// check whether this file was deleted
		if ((raw_inode->i_flags&0x1)==((raw_inode->i_flags&0x2)>>1)) {
			if(raw_inode->i_id==ino) latest_matching_offset=cur_link;
		}

		cur_link=raw_inode->i_link;
	}

	if(latest_matching_offset!=0xffffffff)
	{
		nErr = FlashDevRead(&device_info, latest_matching_offset, J4FS_BASIC_UNIT_SIZE, buf);
		if (nErr != 0) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: error(nErr=0x%x)\n",__FUNCTION__,__LINE__,nErr));
	   		goto error1;
		}

		raw_inode = (struct j4fs_inode *)buf;
		return raw_inode;
	}

Einval:
	J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: error(bad inode number: %lu)\n",__FUNCTION__,__LINE__,(unsigned long) ino));
	kfree(buf);
	return ERR_PTR(-EINVAL);

error1:
	kfree(buf);
	return NULL;

}

void j4fs_read_inode (struct inode * inode)
{
	struct j4fs_inode_info *ei = J4FS_I(inode);
	ino_t ino = inode->i_ino;
	struct j4fs_inode * raw_inode;

	if(j4fs_panic==1) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: j4fs panic\n",__FUNCTION__,__LINE__));
		return;
	}

	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));

	// root inode
	if(ino==J4FS_ROOT_INO)
	{
		inode->i_size = 0;
		inode->i_mode=S_IFDIR|S_IWUSR|S_IRUGO|S_IXUGO;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 29))
	inode->i_uid=current->fsuid;
	inode->i_gid=current->fsgid;
#else
	inode->i_uid=current->cred->fsuid;
	inode->i_gid=current->cred->fsgid;
#endif

		strcpy(ei->i_filename,"/");
		ei->i_link=device_info.j4fs_offset;
		ei->i_type=J4FS_FILE_TYPE;
		ei->i_flags=0x3;
		ei->i_id=J4FS_ROOT_INO;
		ei->i_length=0;

		inode->i_op = &j4fs_dir_inode_operations;
		inode->i_mapping->a_ops = &j4fs_aops;
		inode->i_fop = &j4fs_dir_operations;

		return;
	}

	raw_inode = j4fs_get_inode(inode->i_sb, ino);

	if (IS_ERR(raw_inode))
 		goto bad_inode;

	inode->i_size = le32_to_cpu(raw_inode->i_length);
	inode->i_mode=S_IFREG|S_IWUGO|S_IRUGO|S_IXUGO;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 29))
	inode->i_uid=current->fsuid;
	inode->i_gid=current->fsgid;
#else
	inode->i_uid=current->cred->fsuid;
	inode->i_gid=current->cred->fsgid;
#endif

	strcpy(ei->i_filename,raw_inode->i_filename);
	ei->i_link=raw_inode->i_link;
	ei->i_type=raw_inode->i_type;
	ei->i_flags=raw_inode->i_flags;
	ei->i_id=raw_inode->i_id;
	ei->i_length=raw_inode->i_length;

	if (S_ISREG(inode->i_mode)) {
		inode->i_op = &j4fs_file_inode_operations;
		inode->i_mapping->a_ops = &j4fs_aops;
		inode->i_fop = &j4fs_file_operations;
	}

	kfree(raw_inode);
	inode->i_flags |= S_SYNC;
	return;

bad_inode:
	make_bad_inode(inode);
}

// TODO : Consider 'dir'
ino_t j4fs_inode_by_name(struct inode * dir, struct dentry *dentry)
{
	unsigned int cur_link;
	struct j4fs_inode_info *ei = J4FS_I(dir);
	struct j4fs_inode *raw_inode;
	ino_t ino;
	int nErr;
	BYTE *buf;

	if(j4fs_panic==1) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: j4fs panic\n",__FUNCTION__,__LINE__));
		return 0;
	}

	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));

	buf=kmalloc(J4FS_BASIC_UNIT_SIZE,GFP_NOFS);

	cur_link=ei->i_link;
	while(cur_link!=0xffffffff)
	{
		// check the partition range
		j4fs_check_partition_range(cur_link);

		nErr = FlashDevRead(&device_info, cur_link, J4FS_BASIC_UNIT_SIZE, buf);
		if (nErr != 0) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: error(nErr=0x%x)\n",__FUNCTION__,__LINE__,nErr));
	   		goto error1;
		}

		raw_inode = (struct j4fs_inode *) buf;

		//This j4fs_header cannot be interpreted. It means there are no files in this partition(this can happen and this is a normal case) or
		//this j4fs partition is crashed(this should not happen).
		if(raw_inode->i_type!=J4FS_FILE_TYPE)
		{
			// There are no files in this partition or this first j4fs_header is crashed. So, this case should not happen and/or should be repaired.
			if(cur_link==ei->i_link) {
				j4fs_panic("There are no files in this partition or this first j4fs_header is crashed. So, this case should not happen and/or should be repaired.");
				goto error1;
			}

			// This j4fs partition is crashed by some abnormal cause. This should not happen and should be repaired.
			j4fs_panic("this j4fs partition is crashed by some abnormal cause.  This should not happen and should be repaired.");
			goto error1;
		}

		// check whether this file was deleted
		if ((raw_inode->i_flags&0x1)==((raw_inode->i_flags&0x2)>>1)) {
			if(!strcmp(raw_inode->i_filename, dentry->d_name.name))
			{
				ino = raw_inode->i_id;
				kfree(buf);
				return ino;
			}
		}

		cur_link=raw_inode->i_link;
	}

error1:
	kfree(buf);

	return 0;

}

DWORD valid_offset[128][2];
int j4fs_readdir (struct file * filp, void * dirent, filldir_t filldir)
{
	unsigned int curoffs, offset, cur_link;
	struct inode *inode = filp->f_dentry->d_inode;
	struct j4fs_inode_info *ei = J4FS_I(inode);
	struct j4fs_inode *raw_inode;
	int i,j, nErr;
	BYTE *buf;
	int count=0;

	if(j4fs_panic==1) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: j4fs panic\n",__FUNCTION__,__LINE__));
		return 0;
	}

	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));

	buf=kmalloc(J4FS_BASIC_UNIT_SIZE,GFP_NOFS);

	j4fs_GrossLock();

	offset = filp->f_pos;

	if (offset == 0) {
		nErr=filldir(dirent, ".", 1, offset, filp->f_dentry->d_inode->i_ino, DT_DIR);
		if (nErr != 0) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: error(nErr=0x%x)\n",__FUNCTION__,__LINE__,nErr));
	   		goto error1;
		}
		offset++;
		filp->f_pos++;
	}

	if (offset == 1) {
		nErr=filldir(dirent, "..", 2, offset,filp->f_dentry->d_parent->d_inode->i_ino, DT_DIR);
		if (nErr != 0) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: error(nErr=0x%x)\n",__FUNCTION__,__LINE__,nErr));
	   		goto error1;
		}
		offset++;
		filp->f_pos++;
	}

	curoffs = 1;

	cur_link=ei->i_link;
	while(cur_link!=0xffffffff)
	{
		// check the partition range
		j4fs_check_partition_range(cur_link);

		nErr = FlashDevRead(&device_info, cur_link, J4FS_BASIC_UNIT_SIZE, buf);
		if (nErr != 0) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: error(nErr=0x%x)\n",__FUNCTION__,__LINE__,nErr));
	   		goto error1;
		}

		raw_inode = (struct j4fs_inode *) buf;

		//This j4fs_header cannot be interpreted. It means there are no files in this partition(this can happen and this is a normal case) or
		//this j4fs partition is crashed(this should not happen).
		if(raw_inode->i_type!=J4FS_FILE_TYPE)
		{
			// There are no files in this partition or this first j4fs_header is crashed. So, this case should not happen and/or should be repaired.
			if(cur_link==ei->i_link) {
				j4fs_panic("There are no files in this partition or this first j4fs_header is crashed. So, this case should not happen and/or should be repaired.");
				goto error1;
			}

			// This j4fs partition is crashed by some abnormal cause. This should not happen and should be repaired.
			j4fs_panic("this j4fs partition is crashed by some abnormal cause.  This should not happen and should be repaired.");
			goto error1;
		}

		// check whether this file was deleted
		if ((raw_inode->i_flags&0x1)==((raw_inode->i_flags&0x2)>>1)) {
			valid_offset[count][0]=raw_inode->i_id;
			valid_offset[count][1]=cur_link;
			count++;
		}
		cur_link=raw_inode->i_link;
	}

	// TODO: we exclude last object header because last object header can't decide invalidation of old files. Is it right ???
	// Add files(latest valid object) to directory entry
	for(i=0;i<count;i++)
	{
		for(j=i+1;j<count;j++)
		{
			// If inode number is same, valid_offset[i] is invalid
			if(valid_offset[i][0]==valid_offset[j][0]) break;
		}

		// Add latest valid object to directory entry
		if(j==count)
		{
			curoffs++;
			if(curoffs >= offset)
			{
				nErr = FlashDevRead(&device_info, valid_offset[i][1], J4FS_BASIC_UNIT_SIZE, buf);
				if (nErr != 0) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: error(nErr=0x%x)\n",__FUNCTION__,__LINE__,nErr));
			   		goto error1;
				}

				raw_inode = (struct j4fs_inode *) buf;

				nErr=filldir(dirent, raw_inode->i_filename, strlen(raw_inode->i_filename), offset, raw_inode->i_id, DT_REG);

				if(nErr <0) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: error(nErr=0x%08x,filename=%s, file length=%d)\n",__FUNCTION__,__LINE__,nErr,raw_inode->i_filename, strlen(raw_inode->i_filename)));
					goto error1;
				}
				else
				{
					J4FS_T(J4FS_TRACE_FS,("%s %d: success(filename=%s, file length=%d)\n",__FUNCTION__,__LINE__,raw_inode->i_filename, strlen(raw_inode->i_filename)));
					offset++;
					filp->f_pos++;
				}
			}
		}
	}

error1:
	kfree(buf);
	j4fs_GrossUnlock();
	return 0;
}



#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
int j4fs_permission(struct inode *inode, int mask)
#else
int j4fs_permission(struct inode *inode, int mask, struct nameidata *nd)
#endif
{
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
struct inode *j4fs_iget(struct super_block *sb, unsigned long ino)
{
	struct inode * inode;
	inode = iget_locked(sb, ino);

	if (!inode)
		return ERR_PTR(-ENOMEM);

	if (!(inode->i_state & I_NEW))
		return inode;

	j4fs_read_inode(inode);
	unlock_new_inode(inode);
	return inode;
}
#endif

struct dentry *j4fs_lookup(struct inode * dir, struct dentry *dentry, struct nameidata *nd)
{
	struct inode * inode;
	ino_t ino;

	J4FS_T(J4FS_TRACE_FS,("%s %d:(filename=%s)\n",__FUNCTION__,__LINE__,dentry->d_name.name));

	if (dentry->d_name.len > J4FS_NAME_LEN)
		return ERR_PTR(-ENAMETOOLONG);

	ino = j4fs_inode_by_name(dir, dentry);
	inode = NULL;
	if (ino) {
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
		inode = j4fs_iget(dir->i_sb, ino);
	#else
		inode = iget(dir->i_sb, ino);
	#endif
		if (!inode)
			return ERR_PTR(-EACCES);
	}
	return d_splice_alias(inode, dentry);
}

struct inode *j4fs_new_inode(struct inode *dir, struct dentry *dentry, int mode)
{
	struct super_block *sb;
	struct inode * inode;
	struct j4fs_inode_info *ei;
	unsigned int offset, last_object_offset=0xffffffff, new_object_offset=0xffffffff;
	struct j4fs_inode *raw_inode=0;
	ino_t ino = J4FS_FIRST_INO-1;
	int nErr;
	BYTE *buf;

#ifdef J4FS_TRANSACTION_LOGGING
	j4fs_transaction *transaction;
#endif

	if(j4fs_panic==1) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: j4fs panic\n",__FUNCTION__,__LINE__));
		return NULL;
	}

	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));

	// allocate new inode
	sb = dir->i_sb;
	inode = new_inode(sb);
	if (!inode) {
		return ERR_PTR(-ENOMEM);
	}

	buf=kmalloc(J4FS_BASIC_UNIT_SIZE,GFP_NOFS);

#ifdef J4FS_TRANSACTION_LOGGING
	transaction=kmalloc(J4FS_TRANSACTION_SIZE,GFP_NOFS);
#endif

	ei = J4FS_I(inode);

	if(is_invalid_j4fs_rw_start())
	{
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error! j4fs_rw_start is invalid(j4fs_rw_start=0x%08x, j4fs_end=0x%08x, ro_j4fs_header_count=0x%08x)\n",
			__FUNCTION__, __LINE__, j4fs_rw_start, device_info.j4fs_end, ro_j4fs_header_count));
		j4fs_panic("j4fs_rw_start is invalid");
		goto error1;
	}

	// find existing largest inode number
	// TODO: 1. RO files --> use ro_j4fs_header buffer
	offset=device_info.j4fs_offset;
	while(offset!=0xffffffff)
	{
		// check the partition range
		j4fs_check_partition_range(offset);

		// read j4fs_header
		nErr = FlashDevRead(&device_info, offset, J4FS_BASIC_UNIT_SIZE, buf);
		if (nErr != 0) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: error(nErr=0x%x)\n",__FUNCTION__,__LINE__,nErr));
	   		goto error1;
		}

		raw_inode 	= (struct j4fs_inode *)buf;

		//This j4fs_header cannot be interpreted. It means there are no files in this partition(this can happen and this is a normal case) or
		//this j4fs partition is crashed(this should not happen).
		if(raw_inode->i_type!=J4FS_FILE_TYPE)
		{
			// There are no files in this partition or this first j4fs_header is crashed. So, this case should not happen and/or should be repaired.
			if(offset==device_info.j4fs_offset) {
				j4fs_panic("There are no files in this partition or this first j4fs_header is crashed. So, this case should not happen and/or should be repaired.");
				goto error1;;
			}

			// This j4fs partition is crashed by some abnormal cause. This should not happen and should be repaired.
			j4fs_panic("this j4fs partition is crashed by some abnormal cause.  This should not happen and should be repaired.");
			goto error1;
		}

		// check whether this file was deleted
		if ((raw_inode->i_flags&0x1)==((raw_inode->i_flags&0x2)>>1)) {
			if(raw_inode->i_id>ino) ino=raw_inode->i_id;
		}

		last_object_offset=offset;
		offset=raw_inode->i_link;
	}

	// set inode number
	ino++;

	ei->i_id=ino;
	inode->i_ino = ino;
	inode->i_mode=S_IFREG|S_IWUGO|S_IRUGO|S_IXUGO;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 29))
	inode->i_uid=current->fsuid;
	inode->i_gid=current->fsgid;
#else
	inode->i_uid=current->cred->fsuid;
	inode->i_gid=current->cred->fsgid;
#endif
	inode->i_op = &j4fs_file_inode_operations;
	inode->i_mapping->a_ops = &j4fs_aops;
	inode->i_fop = &j4fs_file_operations;
	inode->i_flags |= S_SYNC;

	if(last_object_offset!=0xffffffff)
	{
		J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));
		new_object_offset=last_object_offset;
		new_object_offset+=J4FS_BASIC_UNIT_SIZE;	// j4fs_header
		new_object_offset+=raw_inode->i_length;	// data
		new_object_offset=(new_object_offset+J4FS_BASIC_UNIT_SIZE-1)/J4FS_BASIC_UNIT_SIZE*J4FS_BASIC_UNIT_SIZE;	// J4FS_BASIC_UNIT_SIZE align
	}
	else	//there are no files in this partition, so write first offset of partition
	{
		j4fs_panic("There are no files in this partition. There should be one file in j4fs file system at least");
		goto error1;
	}

	if((new_object_offset+J4FS_BASIC_UNIT_SIZE-1)>device_info.j4fs_end)
	{
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: partition size overflow(new_object_offset=0x%08x, j4fs_end=0x%08x)\n",__FUNCTION__,__LINE__,new_object_offset,device_info.j4fs_end));
		goto error1;
	}

#ifdef J4FS_TRANSACTION_LOGGING
	// setting transaction variable
	memset(transaction,0xff,J4FS_TRANSACTION_SIZE);
	transaction->magic=J4FS_MAGIC;
	transaction->sequence=j4fs_next_sequence++;
	transaction->ino=ino;
	memcpy(transaction->filename,dentry->d_name.name,dentry->d_name.len);
	transaction->filename[dentry->d_name.len]=0;
	transaction->opcode=J4FS_LAST_OBJECT_CREATE_STEP1;
	transaction->offset=new_object_offset;
	transaction->b_link=transaction->a_link=0xffffffff;
	transaction->b_length=transaction->a_length=0;

	nErr = FlashDevWrite(&device_info, j4fs_transaction_next_offset, J4FS_TRANSACTION_SIZE, (BYTE *)transaction);

	if (error(nErr)) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: error(nErr=0x%x)\n",__FUNCTION__,__LINE__,nErr));
   		goto error1;
	}

	j4fs_transaction_next_offset+=J4FS_TRANSACTION_SIZE;
	if(j4fs_transaction_next_offset>=device_info.j4fs_device_end) j4fs_transaction_next_offset=device_info.j4fs_end;
#endif

	// add new object(j4fs_header)
	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));
	memset(buf, 0xff, J4FS_BASIC_UNIT_SIZE);
	raw_inode = (struct j4fs_inode *)buf;
	raw_inode->i_link=0xffffffff;
	raw_inode->i_type=J4FS_FILE_TYPE;
	raw_inode->i_flags=0x3;
	raw_inode->i_id=ino;
	raw_inode->i_length=0;
	memcpy(raw_inode->i_filename, dentry->d_name.name, dentry->d_name.len);
	raw_inode->i_filename[dentry->d_name.len]=0;

	nErr = FlashDevWrite(&device_info, new_object_offset, J4FS_BASIC_UNIT_SIZE, buf);
	if (nErr != 0) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: error(nErr=0x%x)\n",__FUNCTION__,__LINE__,nErr));
   		goto error1;
	}

	// update last_inode
	if(last_object_offset!=0xffffffff)
	{
		J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));
		nErr = FlashDevRead(&device_info, last_object_offset, J4FS_BASIC_UNIT_SIZE, buf);
		if (nErr != 0) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: error(nErr=0x%x)\n",__FUNCTION__,__LINE__,nErr));
	   		goto error1;
		}

		raw_inode = (struct j4fs_inode *)buf;


	#ifdef J4FS_TRANSACTION_LOGGING
		// setting transaction variable
		memset(transaction,0xff,J4FS_TRANSACTION_SIZE);
		transaction->magic=J4FS_MAGIC;
		transaction->sequence=j4fs_next_sequence++;
		transaction->ino=raw_inode->i_id;
		strcpy(transaction->filename,raw_inode->i_filename);
		transaction->opcode=J4FS_LAST_OBJECT_CREATE_STEP2;
		transaction->offset=last_object_offset;
		transaction->b_link=raw_inode->i_link;
		transaction->a_link=new_object_offset;
		transaction->b_length=transaction->a_length=raw_inode->i_length;

		nErr = FlashDevWrite(&device_info, j4fs_transaction_next_offset, J4FS_TRANSACTION_SIZE, (BYTE *)transaction);

		if (error(nErr)) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: error(nErr=0x%x)\n",__FUNCTION__,__LINE__,nErr));
	   		goto error1;
		}

		j4fs_transaction_next_offset+=J4FS_TRANSACTION_SIZE;
		if(j4fs_transaction_next_offset>=device_info.j4fs_device_end) j4fs_transaction_next_offset=device_info.j4fs_end;
	#endif

		raw_inode->i_link=new_object_offset;

		nErr = FlashDevWrite(&device_info, last_object_offset, J4FS_BASIC_UNIT_SIZE, buf);
		if (nErr != 0) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: error(nErr=0x%x)\n",__FUNCTION__,__LINE__,nErr));
	   		goto error1;
		}
	}

	kfree(buf);
	return inode;

error1:
	kfree(buf);
#ifdef J4FS_TRANSACTION_LOGGING
	kfree(transaction);
#endif
	return NULL;
}


/*
 *	Parent is locked.
 */
int j4fs_add_link (struct dentry *dentry, struct inode *inode)
{
	// write j4fs_header
	// TODO
	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));

	return 0;

}

int j4fs_add_nondir(struct dentry *dentry, struct inode *inode)
{
	int err;

	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));

	err= j4fs_add_link(dentry, inode);
	if (!err) {
		d_instantiate(dentry, inode);
		return 0;
	}
	inode_dec_link_count(inode);
	iput(inode);
	return err;
}

/*
 * By the time this is called, we already have created
 * the directory cache entry for the new file, but it
 * is so far negative - it has no inode.
 * This function is call by do_filp_open()->open_namei()->open_namei_create()->vfs_create()->dir->i_op->create()
 *
 * If the create succeeds, we fill in the inode information
 * with d_instantiate().
 */
int j4fs_create (struct inode * dir, struct dentry * dentry, int mode, struct nameidata *nd)
{
	struct inode * inode;
	int err=-1;

	if(j4fs_panic==1) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: j4fs panic\n",__FUNCTION__,__LINE__));
		return err;
	}

	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));

	inode = j4fs_new_inode(dir, dentry, mode);

	if (!IS_ERR(inode)) {
		inode->i_op = &j4fs_file_inode_operations;
		inode->i_mapping->a_ops = &j4fs_aops;
		inode->i_fop = &j4fs_file_operations;

		//mark_inode_dirty(inode);
		err = j4fs_add_nondir(dentry, inode);
	}
	return err;
}

/* Space holding and freeing is done to ensure we have space available for write_begin/end */
/* For now we just assume few parallel writes and check against a small number. */
/* Todo: need to do this with a counter to handle parallel reads better */

int j4fs_hold_space(int size)
{
	unsigned int offset, last_object_offset=0xffffffff, new_object_offset=0xffffffff;
	struct j4fs_inode *raw_inode=NULL;
	int nErr;
	BYTE *buf;

	if(j4fs_panic==1) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: j4fs panic\n",__FUNCTION__,__LINE__));
		return 0;
	}

	buf=kmalloc(J4FS_BASIC_UNIT_SIZE,GFP_NOFS);

	// find existing largest inode number
	offset=device_info.j4fs_offset;
	while(offset!=0xffffffff)
	{
		// check the partition range
		j4fs_check_partition_range(offset);

		// read j4fs_header
		nErr = FlashDevRead(&device_info, offset, J4FS_BASIC_UNIT_SIZE, buf);
		if (nErr != 0) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: error(nErr=0x%x)\n",__FUNCTION__,__LINE__,nErr));
	   		goto error1;
		}

		raw_inode = (struct j4fs_inode *)buf;

		//This j4fs_header cannot be interpreted. It means there are no files in this partition(this can happen and this is a normal case) or
		//this j4fs partition is crashed(this should not happen).
		if(raw_inode->i_type!=J4FS_FILE_TYPE)
		{
			// There are no files in this partition or this first j4fs_header is crashed. So, this case should not happen and/or should be repaired.
			if(offset==device_info.j4fs_offset) {
				j4fs_panic("There are no files in this partition or this first j4fs_header is crashed. So, this case should not happen and/or should be repaired.");
				goto error1;
			}

			// This j4fs partition is crashed by some abnormal cause. This should not happen and should be repaired.
			j4fs_panic("this j4fs partition is crashed by some abnormal cause.  This should not happen and should be repaired.");
			goto error1;
		}

		last_object_offset=offset;
		offset=raw_inode->i_link;
	}

	if(last_object_offset!=0xffffffff)
	{
		J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));
		new_object_offset=last_object_offset;
		new_object_offset+=J4FS_BASIC_UNIT_SIZE;	// j4fs_header
		new_object_offset+=raw_inode->i_length;	// data
		new_object_offset=(new_object_offset+J4FS_BASIC_UNIT_SIZE-1)/J4FS_BASIC_UNIT_SIZE*J4FS_BASIC_UNIT_SIZE;	// 4096 align
	}

	kfree(buf);
	if((new_object_offset+size-1)>device_info.j4fs_end) return 0;
	else return 1;

error1:
	kfree(buf);
	return 0;
}

int j4fs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct j4fs_sb_info * sbi;
	struct j4fs_super_block * es;
	struct inode *root;
	u32 ret;

	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));

	sbi = kzalloc(sizeof(*sbi), GFP_NOFS);
	if (!sbi)
		return -ENOMEM;

	es=kzalloc(sizeof(*es), GFP_NOFS);
	if (!es)
	{
		kfree(sbi);	
		return -ENOMEM;
	}

	sb->s_fs_info = sbi;
	sbi->s_es = es;
	sbi->s_first_ino=J4FS_FIRST_INO;

	// Block size is J4FS_BASIC_UNIT_SIZE (4096)
	sb->s_flags = 0;
	sb->s_maxbytes = 0xffffffff;

	sb->s_blocksize=J4FS_BASIC_UNIT_SIZE;
	sb->s_blocksize_bits=J4FS_BASIC_UNIT_SIZE_BITS;
	sb->s_op = &j4fs_sops;
	sb->s_xattr = NULL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
	root=j4fs_iget(sb, J4FS_ROOT_INO);
#else
	root = iget(sb, J4FS_ROOT_INO);
#endif

	sb->s_root = d_alloc_root(root);

	// Set device_info.j4fs_end using STLInfo.nTotalLogScts
#if defined(J4FS_USE_XSR)
	ret = STL_IOCtl(0, device_info.device, STL_IOCTL_LOG_SECTS, NULL, sizeof(u32), &tmp, sizeof(u32), &len); 
#elif defined(J4FS_USE_FSR)
	ret = FSR_STL_IOCtl(0, device_info.device, FSR_STL_IOCTL_LOG_SECTS, NULL, sizeof(u32), &tmp, sizeof(u32), &len); 
#endif

#if defined(J4FS_USE_XSR) || defined(J4FS_USE_FSR)
	if (ret) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(0x%08x)\n",__FUNCTION__,__LINE__,ret));
		return -EINVAL;
	}

	// j4fs transaction(1 block) for fils system crash debugging
	// J4FS_BASIC_UNIT_SIZE align
	device_info.j4fs_device_end=((tmp/(J4FS_BASIC_UNIT_SIZE/512))*(J4FS_BASIC_UNIT_SIZE/512))*512-1;
	device_info.j4fs_end=device_info.j4fs_device_end-PHYSICAL_BLOCK_SIZE;

// J4FS for moviNAND merged from ROSSI
#elif defined(J4FS_USE_MOVI)
	// j4fs transaction(1 block) for fils system crash debugging
	// J4FS_BASIC_UNIT_SIZE align
	device_info.j4fs_device_end=(((J4FS_BLOCK_COUNT * 512)/(J4FS_BASIC_UNIT_SIZE/512))*(J4FS_BASIC_UNIT_SIZE/512))*512-1;
	device_info.j4fs_end=device_info.j4fs_device_end-PHYSICAL_BLOCK_SIZE;
#endif
// J4FS for moviNAND merged from ROSSI

	J4FS_T(J4FS_TRACE_FS,("%s %d: device_info.j4fs_end=0x%08x, device_info.j4fs_device_end=0x%08x\n",__FUNCTION__,__LINE__,device_info.j4fs_end,device_info.j4fs_device_end));

	if (!sb->s_root) {
		iput(root);
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: error\n",__FUNCTION__,__LINE__));
		goto failed;
	}

	sema_init(&device_info.grossLock, 1);

#ifdef J4FS_TRANSACTION_LOGGING
	ret=fsd_initialize_transaction();

	if (error(ret)) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
   		goto failed;
	}

	J4FS_T(J4FS_TRACE_FS,("%s %d: j4fs_next_sequence=0x%08x, j4fs_transaction_next_offset=0x%08x\n",__FUNCTION__,__LINE__,j4fs_next_sequence,j4fs_transaction_next_offset));
#endif

	ret=fsd_reclaim();

	if (error(ret)) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
   		goto failed;
	}

	return 0;

failed:
	sb->s_fs_info = NULL;
	kfree(sbi);
	kfree(es);
	return -EINVAL;
}

struct dentry* j4fs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data)
{
	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));

	return mount_bdev(fs_type, flags, dev_name, data, j4fs_fill_super);
}

struct kmem_cache * j4fs_inode_cachep;

struct inode *j4fs_alloc_inode(struct super_block *sb)
{
	struct j4fs_inode_info *ei;

	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));

	ei = (struct j4fs_inode_info *)kmem_cache_alloc(j4fs_inode_cachep, GFP_NOFS);
	if (!ei)
		return NULL;

	ei->vfs_inode.i_version = 1;
	return &ei->vfs_inode;
}

void j4fs_destroy_inode(struct inode *inode)
{
	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));

	kmem_cache_free(j4fs_inode_cachep, J4FS_I(inode));
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
void init_once(void *foo)
#else
void init_once(struct kmem_cache * cachep, void *foo)
#endif
{
	struct j4fs_inode_info *ei = (struct j4fs_inode_info *) foo;

	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));

	rwlock_init(&ei->i_meta_lock);
	inode_init_once(&ei->vfs_inode);
}

int init_inodecache(void)
{
	j4fs_inode_cachep = kmem_cache_create("j4fs_inode_cache",
					     sizeof(struct j4fs_inode_info),
					     0, (SLAB_RECLAIM_ACCOUNT|
						SLAB_MEM_SPREAD),
					     init_once);
	if (j4fs_inode_cachep == NULL)
		return -ENOMEM;
	return 0;
}

void destroy_inodecache(void)
{
	kmem_cache_destroy(j4fs_inode_cachep);
}

struct file_system_type j4fs_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "j4fs",
	.mount		= j4fs_mount,
	.kill_sb	= kill_block_super,
	.fs_flags	= FS_REQUIRES_DEV,
};

extern ssize_t (*lfs_read_module)(struct file *file, const char __user * buffer, size_t count, loff_t *ppos);
extern ssize_t (*lfs_write_module)(struct file *file, const char __user * buffer, size_t count, loff_t *ppos);
extern unsigned int j4fs_PORMask;
extern unsigned int j4fs_PORCount;

ssize_t lfs_read(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	printk("%s %d: (j4fs_PORMask,j4fs_PORCount)=(%x,%d)\n",__FUNCTION__,__LINE__,j4fs_PORMask, j4fs_PORCount);
	return 0;
}

ssize_t lfs_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
	char kbuf[1024];

	if (copy_from_user(&kbuf, buffer, count))
		return -EFAULT;

	if (sscanf(kbuf, "%x %d", &j4fs_PORMask, &j4fs_PORCount) != 2)
		return -EINVAL;

	printk("%s %d: (j4fs_PORMask,j4fs_PORCount)=(%x,%d)\n",__FUNCTION__,__LINE__,j4fs_PORMask, j4fs_PORCount);
	return -EINVAL;
}

int j4fs_fsync(struct file *file, int datasync)
{
	return 0;
}

int __init init_j4fs_fs(void)
{
	int err;
	j4fs_ctrl ctl;

	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));

	err = init_inodecache();
	if (err)
		goto out1;

	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));

        err = register_filesystem(&j4fs_fs_type);
	if (err)
		goto out;

#if 0
	lfs_read_module=lfs_read;
	lfs_write_module=lfs_write;
#endif
	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));

	// Initialize j4fs_device_info
	ctl.scmd=J4FS_INIT;
	fsd_special(&ctl);

	J4FS_T(J4FS_TRACE_FS,("%s %d\n",__FUNCTION__,__LINE__));

	return 0;
out:
	destroy_inodecache();
out1:
	return err;
}

void __exit exit_j4fs_fs(void)
{
// J4FS for moviNAND merged from ROSSI
#if defined(J4FS_USE_MOVI)
	j4fs_ctrl ctl;
	
	// Exit j4fs_device_info
	ctl.scmd=J4FS_EXIT;
	fsd_special(&ctl);
#endif
// J4FS for moviNAND merged from ROSSI

	unregister_filesystem(&j4fs_fs_type);
	destroy_inodecache();
}

module_init(init_j4fs_fs)
module_exit(exit_j4fs_fs)


const struct address_space_operations j4fs_aops = {
	.readpage		= j4fs_readpage,
	.writepage		= j4fs_writepage,
#if (J4FS_USE_WRITE_BEGIN_END > 0)
	.write_begin = j4fs_write_begin,
	.write_end = j4fs_write_end,
#else
	.prepare_write = j4fs_prepare_write,
	.commit_write = j4fs_commit_write,
#endif
};

const struct file_operations j4fs_file_operations = {
	.read		= do_sync_read,
	.write		= do_sync_write,
	.aio_read	= generic_file_aio_read,
	.aio_write	= generic_file_aio_write,
	.open		= generic_file_open,
	.llseek		= generic_file_llseek,
	.fsync		= j4fs_fsync,
};

const struct file_operations j4fs_dir_operations = {
	.llseek		= generic_file_llseek,
	.read		= generic_read_dir,
	.readdir		= j4fs_readdir,
};

const struct inode_operations j4fs_file_inode_operations = {
	.permission = NULL,
};

const struct inode_operations j4fs_dir_inode_operations = {
	.create		= j4fs_create,
	.lookup		= j4fs_lookup,
	.permission	= NULL,
};

const struct super_operations j4fs_sops = {
	.alloc_inode	= j4fs_alloc_inode,
	.destroy_inode	= j4fs_destroy_inode,

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 28)
	.read_inode	= j4fs_read_inode,
#endif
};

