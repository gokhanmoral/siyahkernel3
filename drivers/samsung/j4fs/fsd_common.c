/*
 * Copyright (C) 2008, 2009,  Samsung Electronics Co. Ltd. All Rights Reserved.
 *       Written by Linux Lab, MITs Development Team, Mobile Communication Division.
 */

/*
 * fsd_common.c
 *
 * File System Driver
 *
 *
 * 2009.02 - First editing by SungHwan.yun <sunghwan.yun@samsung.com> @LDK@
 *
 * 2009.03 - Currently managed by  SungHwan.yun <sunghwan.yun@samsung.com> @LDK@
 *
 */
#include <linux/slab.h>
#include "j4fs.h"

#ifndef __KERNEL__
#include <asm/util.h>
#endif

j4fs_device_info device_info;
unsigned int j4fs_traceMask=0;
unsigned int j4fs_PORMask=0;
unsigned int j4fs_PORCount=0;
unsigned int j4fs_rw_start=0;
j4fs_header ro_j4fs_header[J4FS_MAX_RO_FILES_NUMBER];
int ro_j4fs_header_count=0;
int j4fs_panic=0;

#ifdef J4FS_TRANSACTION_LOGGING
unsigned int j4fs_next_sequence=0;
unsigned int j4fs_transaction_next_offset=0xffffffff;
#endif

int is_invalid_j4fs_rw_start()
{
	if((j4fs_rw_start<device_info.j4fs_offset) ||(j4fs_rw_start>= device_info.j4fs_end) ||
		(ro_j4fs_header_count>=2 &&  ((ro_j4fs_header[ro_j4fs_header_count-2].link+ro_j4fs_header[ro_j4fs_header_count-1].length)>j4fs_rw_start)) ||
		(ro_j4fs_header_count==1 && ((device_info.j4fs_offset+ro_j4fs_header[ro_j4fs_header_count-1].length)>j4fs_rw_start)))
		return 1;
	else return 0;
}

/*
  * This function reads count number of bytes from the file specified by device, type, and ID and places them into 'buffer'.
  * The file must be opened with the OPEN_READ option. The file read begins at the location of the last read or whatever file offset the special seek option set.
  * If during the process there is a failure, the actual field indicates how many bytes were transferred and the status will contain a descriptive error code.
  * parameters :
  *   j4fs_ctrl->buffer
  *   j4fs_ctrl->count
  *   j4fs_ctrl->id
  *   j4fs_ctrl->index
  * return
  *
  */
int fsd_read(j4fs_ctrl *ctl)
{
	DWORD offset, matching_offset=0xffffffff, len, count, file_length=0xffffffff;
	int ret=-1;
	j4fs_header *header;
	int file_exist=0, i;

#ifdef __KERNEL__
	BYTE *buf;
	buf=kmalloc(J4FS_BASIC_UNIT_SIZE,GFP_NOFS);
#else
	BYTE buf[J4FS_BASIC_UNIT_SIZE];
#endif

	J4FS_T(J4FS_TRACE_FSD,("%s %d\n",__FUNCTION__,__LINE__));

	for(i=0;i<ro_j4fs_header_count;i++)
	{
		header = &ro_j4fs_header[i];

		//This j4fs_header cannot be interpreted.
		if(header->type!=J4FS_FILE_TYPE)
		{
			j4fs_panic("This j4fs_header cannot be interpreted. So this j4fs partition is crashed by some abnormal cause.  This should not happen and should be repaired.");
			goto error1;
		}

		// This RO file was deleted and RO file should not be deleted. So this j4fs partition is crashed by some abnormal cause.
		if((header->flags&0x1)!=((header->flags&0x2)>>1))
		{
			j4fs_panic("This RO file was deleted and RO file should not be deleted. So this j4fs partition is crashed by some abnormal cause.  This should be repaired.");
			goto error1;
		}

		// File ID is dismatched, so read next file.
		if(ctl->id && ctl->id!=header->id)
		{
			continue;
		}

		// File ID(inode number) is matched
		#ifdef __KERNEL__
		if( ((ctl->index + ctl->count + PAGE_SIZE-1)/PAGE_SIZE*PAGE_SIZE)
			<= ((header->length + PAGE_SIZE-1)/PAGE_SIZE*PAGE_SIZE) )
		#else
		if( ((ctl->index + ctl->count + J4FS_BASIC_UNIT_SIZE-1)/J4FS_BASIC_UNIT_SIZE*J4FS_BASIC_UNIT_SIZE)
			<= ((header->length + J4FS_BASIC_UNIT_SIZE-1)/J4FS_BASIC_UNIT_SIZE*J4FS_BASIC_UNIT_SIZE) )
		#endif
		{
			matching_offset=(i>0)?ro_j4fs_header[i-1].link:device_info.j4fs_offset;
			file_length=header->length;
		}
		else file_exist=1;

		goto got_header;

	}

	if(is_invalid_j4fs_rw_start())
	{
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error! j4fs_rw_start is invalid(j4fs_rw_start=0x%08x, j4fs_end=0x%08x, ro_j4fs_header_count=0x%08x)\n",
			__FUNCTION__, __LINE__, j4fs_rw_start, device_info.j4fs_end, ro_j4fs_header_count));
		j4fs_panic("j4fs_rw_start is invalid");
		goto error1;
	}

	// the start address of the RW area of the device (partition)
	offset=j4fs_rw_start;

	// find object header corresponding to ctl.id in RW area of the device (partition)
	while(offset!=0xffffffff)
	{
		// check the partition range
		j4fs_check_partition_range(offset);

		// read j4fs_header
		ret = FlashDevRead(&device_info, offset, J4FS_BASIC_UNIT_SIZE, buf);
		if (error(ret)) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
			goto error1;
		}
		header=(j4fs_header *)buf;

		//This j4fs_header cannot be interpreted. It means there are no RW files in this partition(this can happen and this is a normal case) or
		//this j4fs partition is crashed(this should not happen).
		if(header->type!=J4FS_FILE_TYPE)
		{
			// There are no RW files in this partition and this can happen and this is a normal case.
			if(offset==j4fs_rw_start) {
				memset(ctl->buffer,0xff,ctl->count);
				goto error1;
			}

			// This j4fs partition is crashed by some abnormal cause. This should not happen and should be repaired.
			j4fs_panic("this j4fs partition is crashed by some abnormal cause.  This should not happen and should be repaired.");
			goto error1;
		}

		// This file was deleted, so read next j4fs_header.
		if((header->flags&0x1)!=((header->flags&0x2)>>1))
		{
			offset=header->link;
			continue;
		}

		// File ID is dismatched, so read next file.
		if(ctl->id && ctl->id!=header->id)
		{
			offset=header->link;
			continue;
		}

		// File ID is matched. we should read lastest object larger than ctl.index, so go ahead.
		#ifdef __KERNEL__
		if( ((ctl->index + ctl->count + PAGE_SIZE-1)/PAGE_SIZE*PAGE_SIZE)
			<= ((header->length + PAGE_SIZE-1)/PAGE_SIZE*PAGE_SIZE) )
		#else
		if( ((ctl->index + ctl->count + J4FS_BASIC_UNIT_SIZE-1)/J4FS_BASIC_UNIT_SIZE*J4FS_BASIC_UNIT_SIZE)
			<= ((header->length + J4FS_BASIC_UNIT_SIZE-1)/J4FS_BASIC_UNIT_SIZE*J4FS_BASIC_UNIT_SIZE) )
		#endif
		{
			matching_offset=offset;
			file_length=header->length;
		}
		else file_exist=1;

		offset=header->link;
	}

got_header:

	//There is valid objects coressponding to ctl->id
	if(matching_offset!=0xffffffff)
	{
		J4FS_T(J4FS_TRACE_FSD,("%s %d\n",__FUNCTION__,__LINE__));

		// We found the wanted file above. Read the data
		matching_offset+=J4FS_BASIC_UNIT_SIZE;	// j4fs_header takes J4FS_BASIC_UNIT_SIZE
		matching_offset+=ctl->index;	// consider offset
		if(ctl->count==0) len=file_length;
		else len=ctl->count;
		if(len>file_length) len=file_length;
		count=0;

		// read data per page size
		while(len>=device_info.pagesize)
		{
			J4FS_T(J4FS_TRACE_FSD,("%s %d: (offset,count,len)=(0x%08x,%d,%d)\n",__FUNCTION__,__LINE__,matching_offset,count,len));

			ret = FlashDevRead(&device_info, matching_offset, device_info.pagesize, ctl->buffer+count);

			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
				goto error1;
			}
			count+=device_info.pagesize;
			matching_offset+=device_info.pagesize;
			len-=device_info.pagesize;
		}

		// read data less than one page and larger than sector
		if(len>=512)
		{
			J4FS_T(J4FS_TRACE_FSD,("%s %d: (offset,count,len)=(0x%08x,%d,%d)\n",__FUNCTION__,__LINE__,matching_offset,count,len));

			ret = FlashDevRead(&device_info, matching_offset, len/512*512, ctl->buffer+count);

			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
				goto error1;
			}
			count+=(len/512*512);
			matching_offset+=(len/512*512);
			len-=(len/512*512);
		}

		// read remained data less than sector size(512Bytes)
		if(len>0)
		{
			J4FS_T(J4FS_TRACE_FSD,("%s %d: (offset,count,len)=(0x%08x,%d,%d)\n",__FUNCTION__,__LINE__,matching_offset,count,len));

			ret = FlashDevRead(&device_info, matching_offset, 512, buf);

			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
				goto error1;
			}

			memcpy(ctl->buffer+count, buf, len);
			count+=len;
			len=0;
		}

	#ifdef __KERNEL__
		kfree(buf);
	#endif
		return count;
	}
	else//There is no valid object coressponding to ctl->filename.
	{
		memset(ctl->buffer,0xff,ctl->count);
	#ifdef __KERNEL__
		kfree(buf);
	#endif
		if(file_exist) return ctl->count;
		return J4FS_NO_FILE;
	}

error1:
#ifdef __KERNEL__
	kfree(buf);
#endif
	return J4FS_FAIL;
}

/*
  * This function writes count number of bytes from buffer to the file specified by device, type, and ID. The file must be opened with
  * the OPEN_CREATE option. If during the process there is a failure, the actual field indicates how many bytes were transferred and the status
  * will contain a descriptive error code. The file write begins at the location of the last write.
  * parameters :
  *   j4fs_ctrl->buffer
  *   j4fs_ctrl->count
  *   j4fs_ctrl->id
  *   j4fs_ctrl->index
  */
int fsd_write(j4fs_ctrl *ctl)
{
	DWORD offset, last_object_offset=0xffffffff, last_object_length=0xffffffff, matching_latest_object_length=0xffffffff, buffer_index=0, len1, len2;
	DWORD matching_latest_offset=0xffffffff, new_header_offset=0xffffffff, is_it_last_object=0;
	j4fs_header *header = 0;
	int ret=-1;

#ifdef __KERNEL__
	BYTE *buf;
#else
	BYTE buf[J4FS_BASIC_UNIT_SIZE];
#endif

#ifdef J4FS_TRANSACTION_LOGGING
#ifdef __KERNEL__
	j4fs_transaction *transaction;
	transaction=kmalloc(J4FS_TRANSACTION_SIZE,GFP_NOFS);
#else
	BYTE buf1[J4FS_TRANSACTION_SIZE];
	j4fs_transaction *transaction=(j4fs_transaction *)buf1;
#endif
#endif

#ifdef __KERNEL__
	buf=kmalloc(J4FS_BASIC_UNIT_SIZE,GFP_NOFS);
#endif

	J4FS_T(J4FS_TRACE_FSD,("%s %d: (ino,index)=(%d,0x%08x)\n",__FUNCTION__,__LINE__,ctl->id,ctl->index));

	if(is_invalid_j4fs_rw_start())
	{
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error! j4fs_rw_start is invalid(j4fs_rw_start=0x%08x, j4fs_end=0x%08x, ro_j4fs_header_count=0x%08x)\n",
			__FUNCTION__, __LINE__, j4fs_rw_start, device_info.j4fs_end, ro_j4fs_header_count));
		j4fs_panic("j4fs_rw_start is invalid");
		goto error1;
	}

	// parameter checking
	if(ctl->index < 0)
	{
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error! ctl->index is invalid(ctl->index=0x%08x)\n",
			__FUNCTION__, __LINE__, ctl->index));
		j4fs_panic("ctl->index is invalid");
		goto error1;
	}

	if(ctl->count==0)
	{
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: count is zero\n",__FUNCTION__, __LINE__));
	#ifdef __KERNEL__
		kfree(buf);
	#ifdef J4FS_TRANSACTION_LOGGING
		kfree(transaction);
	#endif
	#endif
		return J4FS_SUCCESS;
	}

#ifdef J4FS_TRANSACTION_LOGGING
	// setting transaction variable
	memset(transaction,0xff,J4FS_TRANSACTION_SIZE);
	transaction->count=ctl->count;
	transaction->index=ctl->index;
	transaction->magic=J4FS_MAGIC;
#endif

	// the start address of the RW area of the device (partition)
	offset=j4fs_rw_start;

	// find space(offset) to write this file. offset follows last enry(last_offset,j4fs_header.link==0xffffffff) in the device
	while(offset!=0xffffffff)
	{
		// check the partition range
		j4fs_check_partition_range(offset);

		// read j4fs_header
		ret = FlashDevRead(&device_info, offset, J4FS_BASIC_UNIT_SIZE, buf);
		if (error(ret)) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
			goto error1;
		}
		header=(j4fs_header *)buf;

		//This j4fs_header cannot be interpreted. It means there are no RW files in this partition(this can happen and this is a normal case) or
		//this j4fs partition is crashed(this should not happen).
		if(header->type!=J4FS_FILE_TYPE)
		{
			// There are no RW files in this partition or this first j4fs_header is crashed. Before we write data of new file, user of j4fs should write j4fs_header of new file.
			// So, this case should not happen and/or should be repaired.
			if(offset==j4fs_rw_start) {
				j4fs_panic("There are no RW files in this partition or this first RW j4fs_header is crashed. Before we write data of new file, user of j4fs should write j4fs_header of new file. So, this case should not happen and/or should be repaired..");
				goto error1;
			}

			// This j4fs partition is crashed by some abnormal cause. This should not happen and should be repaired.
			j4fs_panic("this j4fs partition is crashed by some abnormal cause.  This should not happen and should be repaired.");
			goto error1;
		}

		if(header->link==0xffffffff) last_object_offset=offset;

		// we find ID-matching valid file. Because we should find latest file, go ahead.
		if((ctl->id==header->id) && ((header->flags&0x1)==((header->flags&0x2)>>1))) matching_latest_offset = offset;

		offset=header->link;
	}

	// There is no RW files with 'ctl->id' inode number in this partition. Before we write data of new file, user of j4fs should write j4fs_header of new file.
	if( (matching_latest_offset<j4fs_rw_start) ||(matching_latest_offset>device_info.j4fs_end) ) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: matching_latest_offset is invalid(matching_latest_offset=0x%08x)\n",__FUNCTION__,__LINE__,matching_latest_offset));
		j4fs_panic("There are no RW files in this partition. Before we write data of new file, user of j4fs should write j4fs_header of new file.");
		goto error1;
	}

	// last_object_offset is invalid. last_object_offset should be between j4fs_rw_start and device_info.j4fs_end
	if( (last_object_offset<j4fs_rw_start) ||(last_object_offset>device_info.j4fs_end) ) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: last_object_offset is invalid(last_object_offset=0x%08x)\n",__FUNCTION__,__LINE__,last_object_offset));
		j4fs_panic("last_object_offset is invalid. last_object_offset should be between j4fs_rw_start and device_info.j4fs_end");
		goto error1;
	}

	if(matching_latest_offset==last_object_offset) is_it_last_object=1;
	else is_it_last_object=0;

	// when this is a last object in partition (2,3,5,6,8)
	if(is_it_last_object)
	{
		// the length of file which resides at last_object_offset(=matching_latest_offset)
		matching_latest_object_length=header->length;

		if(matching_latest_object_length < ctl->index)		// j4fs don't support file hole
		{
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: j4fs don't support file hole(matching_latest_object_length,ctl->index)=(0x%08x,0x%08x)\n",__FUNCTION__,__LINE__,matching_latest_object_length,ctl->index));
			j4fs_panic("j4fs don't support file hole");
			goto error1;
		}
		else if(matching_latest_object_length == ctl->index)	// when extend file size(2,3,5,6)
		{
			buffer_index=0;
			offset=matching_latest_offset;
			offset+=J4FS_BASIC_UNIT_SIZE;	// header
			offset+=ctl->index;				// data

			// check whether we should reclaim
			if((offset+ctl->count)>device_info.j4fs_end)
			{
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Reclaim is needed(offset,ctl->count,j4fs_end)=(0x%08x,0x%08x,0x%08x)\n",__FUNCTION__,__LINE__,offset,ctl->count,device_info.j4fs_end));
				ret=fsd_reclaim();

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}

			#ifdef __KERNEL__
				kfree(buf);
			#endif
				return J4FS_RETRY_WRITE;
			}

			// write first J4FS_BASIC_UNIT_SIZE-not-aligned data to Storage
			if(ctl->index % J4FS_BASIC_UNIT_SIZE)
			{
				len1 = ctl->index % J4FS_BASIC_UNIT_SIZE;
				len2=J4FS_BASIC_UNIT_SIZE-len1;
				if(len2 > ctl->count) len2=ctl->count;

				// read len1'length' data from Stroage
				ret = FlashDevRead(&device_info, offset-len1, (len1+511)/512*512, buf);
				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}
				memcpy(buf+len1, ctl->buffer, len2);

				// write (len1+len2)'length' data to Stroage
				ret = FlashDevWrite(&device_info, offset-len1, (len1+len2+511)/512*512, buf);
				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}

				buffer_index += len2;
				offset += len2;
				ctl->count -= len2;
			}

			// write data per page size
			while(ctl->count>=device_info.pagesize)
			{
				ret = FlashDevWrite(&device_info, offset, device_info.pagesize, ctl->buffer+buffer_index);

				J4FS_T(J4FS_TRACE_FSD,("%s %d: (buffer_index,offset,ctl->count)=(0x%08x,%d,%d)\n",__FUNCTION__,__LINE__,buffer_index,offset,ctl->count));

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}
				buffer_index+=device_info.pagesize;
				offset+=device_info.pagesize;
				ctl->count-=device_info.pagesize;
			}

			// write data less than one page and larger than sector
			if(ctl->count>=512)
			{
				ret = FlashDevWrite(&device_info, offset, ctl->count/512*512, ctl->buffer+buffer_index);

				J4FS_T(J4FS_TRACE_FSD,("%s %d: (buffer_index,offset,ctl->count)=(0x%08x,%d,%d)\n",__FUNCTION__,__LINE__,buffer_index,offset,ctl->count));

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}
				buffer_index+=(ctl->count/512*512);
				offset+=(ctl->count/512*512);
				ctl->count-=(ctl->count/512*512);
			}

			// write remained data less than sector size(512Bytes)
			if(ctl->count>0)
			{
				memcpy(buf, ctl->buffer+buffer_index, ctl->count);
				ret = FlashDevWrite(&device_info, offset, 512, buf);

				J4FS_T(J4FS_TRACE_FSD,("%s %d: (buffer_index,offset,ctl->count)=(0x%08x,%d,%d)\n",__FUNCTION__,__LINE__,buffer_index,offset,ctl->count));

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}

				buffer_index+=ctl->count;
				offset+=ctl->count;
				ctl->count=0;
			}

			// update existing j4fs_header to extend file size
			// read j4fs_header
			ret = FlashDevRead(&device_info, matching_latest_offset, J4FS_BASIC_UNIT_SIZE, buf);
			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
				goto error1;
			}
			header=(j4fs_header *)buf;

		#ifdef J4FS_TRANSACTION_LOGGING
			// setting transaction variable
			transaction->sequence=j4fs_next_sequence++;
			transaction->ino=header->id;
			strcpy(transaction->filename,header->filename);
			transaction->opcode=J4FS_LAST_OBJECT_WRITE_EXTEND1;
			transaction->offset=matching_latest_offset;
			transaction->b_link=transaction->a_link=header->link;
			transaction->b_length=header->length;
			transaction->a_length=header->length+buffer_index;

			ret = FlashDevWrite(&device_info, j4fs_transaction_next_offset, J4FS_TRANSACTION_SIZE, (BYTE *)transaction);

			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
				goto error1;
			}

			j4fs_transaction_next_offset+=J4FS_TRANSACTION_SIZE;
			if(j4fs_transaction_next_offset>=device_info.j4fs_device_end) j4fs_transaction_next_offset=device_info.j4fs_end;
		#endif

			// update file length
			header->length += buffer_index;

			ret = FlashDevWrite(&device_info, matching_latest_offset, J4FS_BASIC_UNIT_SIZE, buf);

			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
				goto error1;
			}

			goto done;
		}
		else if(matching_latest_object_length > ctl->index)	// when update existing data(8)
		{
			buffer_index=0;
			offset=matching_latest_offset;
			offset+=J4FS_BASIC_UNIT_SIZE;	// header
			offset+=ctl->index;				// data

			// check whether we should reclaim
			if((offset+ctl->count)>device_info.j4fs_end)
			{
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Reclaim is needed(offset,ctl->count,j4fs_end)=(0x%08x,0x%08x,0x%08x)\n",__FUNCTION__,__LINE__,offset,ctl->count,device_info.j4fs_end));
				ret=fsd_reclaim();

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}

			#ifdef __KERNEL__
				kfree(buf);
			#ifdef J4FS_TRANSACTION_LOGGING
				kfree(transaction);
			#endif
			#endif
				return J4FS_RETRY_WRITE;
			}

			// write first J4FS_BASIC_UNIT_SIZE-not-aligned data to Storage
			if(ctl->index % J4FS_BASIC_UNIT_SIZE)
			{
				len1 = ctl->index % J4FS_BASIC_UNIT_SIZE;
				len2=J4FS_BASIC_UNIT_SIZE-len1;
				if(len2 > ctl->count) len2=ctl->count;

				// read len1'length' data from Stroage
				ret = FlashDevRead(&device_info, offset-len1, (len1+511)/512*512, buf);
				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}
				memcpy(buf+len1, ctl->buffer, len2);

				// write (len1+len2)'length' data to Stroage
				ret = FlashDevWrite(&device_info, offset-len1, (len1+len2+511)/512*512, buf);
				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}

				buffer_index += len2;
				offset += len2;
				ctl->count -= len2;
			}

			// write data per page size
			while(ctl->count>=device_info.pagesize)
			{
				ret = FlashDevWrite(&device_info, offset, device_info.pagesize, ctl->buffer+buffer_index);

				J4FS_T(J4FS_TRACE_FSD,("%s %d: (buffer_index,offset,ctl->count)=(0x%08x,%d,%d)\n",__FUNCTION__,__LINE__,buffer_index,offset,ctl->count));

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}
				buffer_index+=device_info.pagesize;
				offset+=device_info.pagesize;
				ctl->count-=device_info.pagesize;
			}

			// write data less than one page and larger than sector
			if(ctl->count>=512)
			{
				ret = FlashDevWrite(&device_info, offset, ctl->count/512*512, ctl->buffer+buffer_index);

				J4FS_T(J4FS_TRACE_FSD,("%s %d: (buffer_index,offset,ctl->count)=(0x%08x,%d,%d)\n",__FUNCTION__,__LINE__,buffer_index,offset,ctl->count));

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}
				buffer_index+=(ctl->count/512*512);
				offset+=(ctl->count/512*512);
				ctl->count-=(ctl->count/512*512);
			}

			// write remained data less than sector size(512Bytes)
			if(ctl->count>0)
			{
				// read 512B'length' data from Stroage
				ret = FlashDevRead(&device_info, offset, 512, buf);
				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}

				memcpy(buf, ctl->buffer+buffer_index, ctl->count);
				ret = FlashDevWrite(&device_info, offset, 512, buf);

				J4FS_T(J4FS_TRACE_FSD,("%s %d: (buffer_index,offset,ctl->count)=(0x%08x,%d,%d)\n",__FUNCTION__,__LINE__,buffer_index,offset,ctl->count));

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}

				buffer_index+=ctl->count;
				offset+=ctl->count;
				ctl->count=0;
			}

			// update existing j4fs_header to update file size if needed
			if((offset-matching_latest_offset-J4FS_BASIC_UNIT_SIZE) > matching_latest_object_length)
			{
				// read j4fs_header
				ret = FlashDevRead(&device_info, matching_latest_offset, J4FS_BASIC_UNIT_SIZE, buf);
				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}
				header=(j4fs_header *)buf;

			#ifdef J4FS_TRANSACTION_LOGGING
				// setting transaction variable
				transaction->sequence=j4fs_next_sequence++;
				transaction->ino=header->id;
				strcpy(transaction->filename,header->filename);
				transaction->opcode=J4FS_LAST_OBJECT_WRITE_EXTEND2;
				transaction->offset=matching_latest_offset;
				transaction->b_link=transaction->a_link=header->link;
				transaction->b_length=header->length;
				transaction->a_length=offset-matching_latest_offset-J4FS_BASIC_UNIT_SIZE;

				ret = FlashDevWrite(&device_info, j4fs_transaction_next_offset, J4FS_TRANSACTION_SIZE, (BYTE *)transaction);

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}

				j4fs_transaction_next_offset+=J4FS_TRANSACTION_SIZE;
				if(j4fs_transaction_next_offset>=device_info.j4fs_device_end) j4fs_transaction_next_offset=device_info.j4fs_end;
			#endif

				header->length = offset-matching_latest_offset-J4FS_BASIC_UNIT_SIZE;
				ret = FlashDevWrite(&device_info, matching_latest_offset, J4FS_BASIC_UNIT_SIZE, buf);

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}
			}
			goto done;
		}
		else
		{
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: we should not come here\n",__FUNCTION__,__LINE__));
			j4fs_panic("we should not come here");
			goto error1;
		}
	}
	else if(!is_it_last_object)
	{
		// the length of file which resides at last_object_offset
		last_object_length = header->length;

		// read j4fs_header
		ret = FlashDevRead(&device_info, matching_latest_offset, J4FS_BASIC_UNIT_SIZE, buf);
		if (error(ret)) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
			goto error1;
		}
		header=(j4fs_header *)buf;
		matching_latest_object_length=header->length;

		if(matching_latest_object_length < ctl->index)		// j4fs don't support file hole
		{
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: j4fs don't support file hole(matching_latest_object_length,ctl->index)=(0x%08x,0x%08x)\n",__FUNCTION__,__LINE__,matching_latest_object_length,ctl->index));
			j4fs_panic("j4fs don't support file hole");
			goto error1;
		}
		else if(matching_latest_object_length >= (ctl->index+ctl->count))	// update not-last object in partition and No extension(7)
		{
			buffer_index=0;
			offset=matching_latest_offset;
			offset+=J4FS_BASIC_UNIT_SIZE;	// header
			offset+=ctl->index;				// data

			// check whether we should reclaim
			if((offset+ctl->count)>device_info.j4fs_end)
			{
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: This case should not happen(offset,ctl->count,j4fs_end)=(0x%08x,0x%08x,0x%08x)\n",__FUNCTION__,__LINE__,offset,ctl->count,device_info.j4fs_end));
				j4fs_panic("This case should not happen");
				goto error1;
			}

			// write first J4FS_BASIC_UNIT_SIZE-not-aligned data to Storage
			if(ctl->index % J4FS_BASIC_UNIT_SIZE)
			{
				len1 = ctl->index % J4FS_BASIC_UNIT_SIZE;
				len2=J4FS_BASIC_UNIT_SIZE-len1;
				if(len2 > ctl->count) len2=ctl->count;

				// read len1'length' data from Stroage
				ret = FlashDevRead(&device_info, offset-len1, (len1+511)/512*512, buf);
				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}
				memcpy(buf+len1, ctl->buffer, len2);

				// write (len1+len2)'length' data to Stroage
				ret = FlashDevWrite(&device_info, offset-len1, (len1+len2+511)/512*512, buf);
				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}

				buffer_index += len2;
				offset += len2;
				ctl->count -= len2;
			}

			// write data per page size
			while(ctl->count>=device_info.pagesize)
			{
				ret = FlashDevWrite(&device_info, offset, device_info.pagesize, ctl->buffer+buffer_index);

				J4FS_T(J4FS_TRACE_FSD,("%s %d: (buffer_index,offset,ctl->count)=(0x%08x,%d,%d)\n",__FUNCTION__,__LINE__,buffer_index,offset,ctl->count));

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}
				buffer_index+=device_info.pagesize;
				offset+=device_info.pagesize;
				ctl->count-=device_info.pagesize;
			}

			// write data less than one page and larger than sector
			if(ctl->count>=512)
			{
				ret = FlashDevWrite(&device_info, offset, ctl->count/512*512, ctl->buffer+buffer_index);

				J4FS_T(J4FS_TRACE_FSD,("%s %d: (buffer_index,offset,ctl->count)=(0x%08x,%d,%d)\n",__FUNCTION__,__LINE__,buffer_index,offset,ctl->count));

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}
				buffer_index+=(ctl->count/512*512);
				offset+=(ctl->count/512*512);
				ctl->count-=(ctl->count/512*512);
			}

			// write remained data less than sector size(512Bytes)
			if(ctl->count>0)
			{
				J4FS_T(J4FS_TRACE_FSD,("%s %d: (buffer_index,offset,ctl->count)=(0x%08x,%d,%d)\n",__FUNCTION__,__LINE__,buffer_index,offset,ctl->count));

				// read 512B'length' data from Stroage
				ret = FlashDevRead(&device_info, offset, 512, buf);
				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}

				memcpy(buf, ctl->buffer+buffer_index, ctl->count);
				ret = FlashDevWrite(&device_info, offset, 512, buf);

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}

				buffer_index+=ctl->count;
				offset+=ctl->count;
				ctl->count=0;
			}

			goto done;
		}
		else if(matching_latest_object_length < (ctl->index+ctl->count))	// update not-last object in partition and file size is extended(9,11,12,13)
		{
			new_header_offset=last_object_offset;
			new_header_offset+=J4FS_BASIC_UNIT_SIZE;	// header
			new_header_offset+=last_object_length;			// data
			new_header_offset=(new_header_offset+J4FS_BASIC_UNIT_SIZE-1)/J4FS_BASIC_UNIT_SIZE*J4FS_BASIC_UNIT_SIZE;	// J4FS_BASIC_UNIT_SIZE Align

			// copy existing 'all data' of 'matching_latest_offset' offset to 'new_header_offset' offset
			len1=matching_latest_object_length;
			len1=(len1+J4FS_BASIC_UNIT_SIZE-1)/J4FS_BASIC_UNIT_SIZE*J4FS_BASIC_UNIT_SIZE;	// J4FS_BASIC_UNIT_SIZE Align
			buffer_index=J4FS_BASIC_UNIT_SIZE;

			// write data per J4FS_BASIC_UNIT_SIZE size
			while(len1>=J4FS_BASIC_UNIT_SIZE)
			{
				J4FS_T(J4FS_TRACE_FSD,("%s %d: (matching_latest_offset,new_header_offset,buffer_index)=(0x%08x,0x%08x,0x%08x)\n",
					__FUNCTION__,__LINE__,matching_latest_offset,new_header_offset,buffer_index));

				ret = FlashDevRead(&device_info, matching_latest_offset+buffer_index, J4FS_BASIC_UNIT_SIZE, buf);

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}

				ret = FlashDevWrite(&device_info, new_header_offset+buffer_index, J4FS_BASIC_UNIT_SIZE, buf);

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}

				buffer_index+=J4FS_BASIC_UNIT_SIZE;
				len1-=J4FS_BASIC_UNIT_SIZE;
			}

			if(len1!=0)
			{
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: This case should not happen(len1)=(0x%08x)\n",__FUNCTION__,__LINE__,len1));
				j4fs_panic("This case should not happen");
				goto error1;
			}

			// copy existing j4fs_header of 'matching_latest_offset' offset to 'new_header_offset' offset
			J4FS_T(J4FS_TRACE_FSD,("%s %d: (matching_latest_offset,new_header_offset)=(0x%08x,0x%08x)\n", __FUNCTION__,__LINE__,matching_latest_offset,new_header_offset));
			ret = FlashDevRead(&device_info, matching_latest_offset, J4FS_BASIC_UNIT_SIZE, buf);
			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
				goto error1;
			}
			header=(j4fs_header *)buf;

		#ifdef J4FS_TRANSACTION_LOGGING
			// setting transaction variable
			transaction->sequence=j4fs_next_sequence++;
			transaction->ino=header->id;
			strcpy(transaction->filename,header->filename);
			transaction->opcode=J4FS_NOLAST_OBJECT_WRITE_EXTEND_STEP1;
			transaction->offset=new_header_offset;
			transaction->b_link=header->link;
			transaction->a_link=0xffffffff;
			transaction->b_length=transaction->a_length=header->length;

			ret = FlashDevWrite(&device_info, j4fs_transaction_next_offset, J4FS_TRANSACTION_SIZE, (BYTE *)transaction);

			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
				goto error1;
			}

			j4fs_transaction_next_offset+=J4FS_TRANSACTION_SIZE;
			if(j4fs_transaction_next_offset>=device_info.j4fs_device_end) j4fs_transaction_next_offset=device_info.j4fs_end;
		#endif

			header->link=0xffffffff;

			ret = FlashDevWrite(&device_info, new_header_offset, J4FS_BASIC_UNIT_SIZE, buf);
			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
				goto error1;
			}

			// update the link of last_object_offset to indicate new_header_offset
			// read j4fs_header
			ret = FlashDevRead(&device_info, last_object_offset, J4FS_BASIC_UNIT_SIZE, buf);
			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
				goto error1;
			}
			header=(j4fs_header *)buf;

		#ifdef J4FS_TRANSACTION_LOGGING
			// setting transaction variable
			transaction->sequence=j4fs_next_sequence++;
			transaction->ino=header->id;
			strcpy(transaction->filename,header->filename);
			transaction->opcode=J4FS_NOLAST_OBJECT_WRITE_EXTEND_STEP2;
			transaction->offset=last_object_offset;
			transaction->b_link=header->link;
			transaction->a_link=new_header_offset;
			transaction->b_length=transaction->a_length=header->length;

			ret = FlashDevWrite(&device_info, j4fs_transaction_next_offset, J4FS_TRANSACTION_SIZE, (BYTE *)transaction);

			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
				goto error1;
			}

			j4fs_transaction_next_offset+=J4FS_TRANSACTION_SIZE;
			if(j4fs_transaction_next_offset>=device_info.j4fs_device_end) j4fs_transaction_next_offset=device_info.j4fs_end;
		#endif

			header->link= new_header_offset;
			ret = FlashDevWrite(&device_info, last_object_offset, J4FS_BASIC_UNIT_SIZE, buf);

			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
				goto error1;
			}

			// write new data(file size is extended)
			buffer_index=0;
			offset=new_header_offset;
			offset+=J4FS_BASIC_UNIT_SIZE;	// header
			offset+=ctl->index;				// data

			// check whether we should reclaim
			if((offset+ctl->count)>device_info.j4fs_end)
			{
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Reclaim is needed(offset,ctl->count,j4fs_end)=(0x%08x,0x%08x,0x%08x)\n",__FUNCTION__,__LINE__,offset,ctl->count,device_info.j4fs_end));
				ret=fsd_reclaim();

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}

			#ifdef __KERNEL__
				kfree(buf);
			#endif
				return J4FS_RETRY_WRITE;
			}

			// write first J4FS_BASIC_UNIT_SIZE-not-aligned data to Storage
			if(ctl->index % J4FS_BASIC_UNIT_SIZE)
			{
				len1=ctl->index % J4FS_BASIC_UNIT_SIZE;
				len2=J4FS_BASIC_UNIT_SIZE-len1;
				if(len2 > ctl->count) len2=ctl->count;

				// read len1'length' data from Stroage
				ret = FlashDevRead(&device_info, offset-len1, (len1+511)/512*512, buf);
				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}
				memcpy(buf+len1, ctl->buffer, len2);

				// write (len1+len2)'length' data to Stroage
				ret = FlashDevWrite(&device_info, offset-len1, (len1+len2+511)/512*512, buf);
				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}

				buffer_index += len2;
				offset += len2;
				ctl->count -= len2;
			}

			// write data per page size
			while(ctl->count>=device_info.pagesize)
			{
				ret = FlashDevWrite(&device_info, offset, device_info.pagesize, ctl->buffer+buffer_index);

				J4FS_T(J4FS_TRACE_FSD,("%s %d: (buffer_index,offset,ctl->count)=(0x%08x,%d,%d)\n",__FUNCTION__,__LINE__,buffer_index,offset,ctl->count));

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}
				buffer_index+=device_info.pagesize;
				offset+=device_info.pagesize;
				ctl->count-=device_info.pagesize;
			}

			// write data less than one page and larger than sector
			if(ctl->count>=512)
			{
				ret = FlashDevWrite(&device_info, offset, ctl->count/512*512, ctl->buffer+buffer_index);

				J4FS_T(J4FS_TRACE_FSD,("%s %d: (buffer_index,offset,ctl->count)=(0x%08x,%d,%d)\n",__FUNCTION__,__LINE__,buffer_index,offset,ctl->count));

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}
				buffer_index+=(ctl->count/512*512);
				offset+=(ctl->count/512*512);
				ctl->count-=(ctl->count/512*512);
			}

			// write remained data less than sector size(512Bytes)
			if(ctl->count>0)
			{
				memcpy(buf, ctl->buffer+buffer_index, ctl->count);
				ret = FlashDevWrite(&device_info, offset, 512, buf);

				J4FS_T(J4FS_TRACE_FSD,("%s %d: (buffer_index,offset,ctl->count)=(0x%08x,%d,%d)\n",__FUNCTION__,__LINE__,buffer_index,offset,ctl->count));

				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
					goto error1;
				}

				buffer_index+=ctl->count;
				offset+=ctl->count;
				ctl->count=0;
			}

			// update new_header_offset to update file size extension
			// read j4fs_header
			ret = FlashDevRead(&device_info, new_header_offset, J4FS_BASIC_UNIT_SIZE, buf);
			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
				goto error1;
			}
			header=(j4fs_header *)buf;

		#ifdef J4FS_TRANSACTION_LOGGING
			// setting transaction variable
			transaction->sequence=j4fs_next_sequence++;
			transaction->ino=header->id;
			strcpy(transaction->filename,header->filename);
			transaction->opcode=J4FS_NOLAST_OBJECT_WRITE_EXTEND_STEP3;
			transaction->offset=new_header_offset;
			transaction->b_link=transaction->a_link=header->link;
			transaction->b_length=header->length;
			transaction->a_length=offset-new_header_offset-J4FS_BASIC_UNIT_SIZE;

			ret = FlashDevWrite(&device_info, j4fs_transaction_next_offset, J4FS_TRANSACTION_SIZE, (BYTE *)transaction);

			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
				goto error1;
			}

			j4fs_transaction_next_offset+=J4FS_TRANSACTION_SIZE;
			if(j4fs_transaction_next_offset>=device_info.j4fs_device_end) j4fs_transaction_next_offset=device_info.j4fs_end;
		#endif

			header->length = offset-new_header_offset-J4FS_BASIC_UNIT_SIZE;
			ret = FlashDevWrite(&device_info, new_header_offset, J4FS_BASIC_UNIT_SIZE, buf);

			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
				goto error1;
			}

			goto done;
		}
		else
		{
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: we should not come here\n",__FUNCTION__,__LINE__));
			j4fs_panic("we should not come here");
			goto error1;
		}
	}
	else
	{
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: we should not come here\n",__FUNCTION__,__LINE__));
		j4fs_panic("we should not come here");
		goto error1;
	}

done:
	J4FS_T(J4FS_TRACE_FSD,("%s %d: write completed(written=%d)\n",__FUNCTION__,__LINE__,buffer_index));

	fsd_print_meta_data();

#ifdef __KERNEL__
	kfree(buf);
#ifdef J4FS_TRANSACTION_LOGGING
	kfree(transaction);
#endif
#endif
	return buffer_index;

error1:
#ifdef __KERNEL__
	kfree(buf);
#ifdef J4FS_TRANSACTION_LOGGING
	kfree(transaction);
#endif
#endif
	return J4FS_FAIL;
}

int fsd_unlink(char *filename)
{
	DWORD offset;
	j4fs_header *header;
	int ret=-1;

#ifdef __KERNEL__
	BYTE *buf;
	buf=kmalloc(J4FS_BASIC_UNIT_SIZE,GFP_NOFS);
#else
	BYTE buf[J4FS_BASIC_UNIT_SIZE];
#endif

	if(filename==NULL) {
	#ifdef __KERNEL__
		kfree(buf);
	#endif
		return 0;
	}

	if(is_invalid_j4fs_rw_start())
	{
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error! j4fs_rw_start is invalid(j4fs_rw_start=0x%08x, j4fs_end=0x%08x, ro_j4fs_header_count=0x%08x)\n",
			__FUNCTION__, __LINE__, j4fs_rw_start, device_info.j4fs_end, ro_j4fs_header_count));
		j4fs_panic("j4fs_rw_start is invalid");
		goto error1;
	}

	// the start address of the RW area of the device (partition)
	offset=j4fs_rw_start;

	// find object header corresponding to filename
	while(offset!=0xffffffff)
	{
		// read j4fs_header
		ret = FlashDevRead(&device_info, offset, J4FS_BASIC_UNIT_SIZE, buf);
		if (error(ret)) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
			goto error1;
		}
		header=(j4fs_header *)buf;

		//This j4fs_header cannot be interpreted. It means there are no RW files in this partition(this can happen and this is a normal case) or
		//this j4fs partition is crashed(this should not happen).
		if(header->type!=J4FS_FILE_TYPE)
		{
			// There are no RW files in this partition or this first j4fs_header is crashed. Before we write data of new file, user of j4fs should write j4fs_header of new file.
			// So, this case should not happen and/or should be repaired.
			if(offset==j4fs_rw_start) {
				j4fs_panic("There are no RW files in this partition or this first RW j4fs_header is crashed. Before we write data of new file, user of j4fs should write j4fs_header of new file. So, this case should not happen and/or should be repaired..");
				goto error1;
			}

			// This j4fs partition is crashed by some abnormal cause. This should not happen and should be repaired.
			j4fs_panic("this j4fs partition is crashed by some abnormal cause.  This should not happen and should be repaired.");
			goto error1;
		}

		// This file was deleted, so read next j4fs_header.
		if((header->flags&0x1)!=((header->flags&0x2)>>1))
		{
			offset=header->link;
			continue;
		}

		// filename is dismatched, so read next file.
		if(strcmp(filename,header->filename))
		{
			offset=header->link;
			continue;
		}

		// this file will be deleted
		header->flags=0x1;

		ret = FlashDevWrite(&device_info, offset, J4FS_BASIC_UNIT_SIZE, buf);
		if (error(ret)) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
	   		goto error1;
		}

		offset=header->link;
	}

	ret=fsd_reclaim();

	if (error(ret)) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
		goto error1;
	}

#ifdef __KERNEL__
	kfree(buf);
#endif
	return 0;

error1:
#ifdef __KERNEL__
	kfree(buf);
#endif
	return J4FS_FAIL;
}

/*
  * This function will reclaim all unused space on the device specified. Reclaim removes file objects that have been marked deleted and
  * relocates valid file objects to create the maximum contiguous free space
  */
int fsd_reclaim()
{
	DWORD offset, rw_start;
	j4fs_mst *mst;
	j4fs_header *header;
	int i;
	int ret=-1;
	int first_unused_area_offset=0xffffffff;

#ifdef __KERNEL__
	BYTE *buf_mst, *buf_header, *buf_data;
#else
	BYTE buf_mst[J4FS_BASIC_UNIT_SIZE], buf_header[J4FS_BASIC_UNIT_SIZE], buf_data[J4FS_BASIC_UNIT_SIZE];
#endif


#ifdef J4FS_TRANSACTION_LOGGING
#ifdef __KERNEL__
	j4fs_transaction *transaction;
	transaction=kmalloc(J4FS_TRANSACTION_SIZE,GFP_NOFS);
#else
	BYTE buf[J4FS_TRANSACTION_SIZE];
	j4fs_transaction *transaction=(j4fs_transaction *)buf;
#endif
#endif

#ifdef __KERNEL__
	buf_mst=kmalloc(J4FS_BASIC_UNIT_SIZE,GFP_NOFS);
	buf_header=kmalloc(J4FS_BASIC_UNIT_SIZE,GFP_NOFS);
	buf_data=kmalloc(J4FS_BASIC_UNIT_SIZE,GFP_NOFS);
#endif

	header=(j4fs_header *)buf_header;
	mst=(j4fs_mst *)buf_mst;

	// read mst
	ret = FlashDevRead(&device_info, 0, J4FS_BASIC_UNIT_SIZE, buf_mst);
	if (error(ret)) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
   		goto error1;
	}

	// If MST is not recognized, Initialize MST
	if(mst->magic!=J4FS_MAGIC)
	{
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: MST is not recognized(mst.magic=0x%08x)\n",__FUNCTION__,__LINE__,mst->magic));
		j4fs_panic("MST is not recognized");
		goto error1;
	}

	if((mst->status|J4FS_PANIC_MASK)==J4FS_PANIC)
	{
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: j4fs is crashed(mst.status=0x%08x)\n",__FUNCTION__,__LINE__,mst->status));
		j4fs_panic("j4fs is crashed");
   		goto error1;
	}

	J4FS_T(J4FS_TRACE_FSD_RECLAIM,("\n%s %d: Reclaim Starts\n",__FUNCTION__,__LINE__));
	fsd_print_meta_data();

	// If reclaim is in progress(sudden power-off in progress of reclaim), restart reclaim. This is power-off-recovery(J4FS_POR)!!!
	if(mst->status&J4FS_RECLAIM_MOVING_DATA_STEP_1) goto moving_data_step_1;
	if(mst->status&J4FS_RECLAIM_MOVING_DATA_STEP_2) goto moving_data_step_2;
	if(mst->status&J4FS_RECLAIM_UPDATE_LINK) goto update_link;

	/**************************************************************************
	 * This is first reclaim since last reclaim is done successfully(There are no power off in the middle of reclaim)
	 **************************************************************************/

	if(!mst->rw_start ||(mst->rw_start>= device_info.j4fs_end))
	{
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error! rw_start is invalid(j4fs_rw_start=0x%08x, j4fs_end=0x%08x)\n", __FUNCTION__, __LINE__, mst->rw_start, device_info.j4fs_end));
		j4fs_panic("rw_start is invalid");
		goto error1;
	}

	// invalidate old valid files
	fsd_mark_invalid();

	// the start address of the RW area of the device (partition)
	offset=mst->rw_start;

	while(offset!=0xffffffff)
	{
		// read j4fs_header
		ret = FlashDevRead(&device_info, offset, J4FS_BASIC_UNIT_SIZE, buf_header);
		if (error(ret)) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
	   		goto error1;
		}

		//This j4fs_header cannot be interpreted. We have scaned all objects in device(partition)
		if(header->type!=J4FS_FILE_TYPE) break;

		// This file is invalid(deleted)
		if((header->flags&0x1)!=((header->flags&0x2)>>1))
		{
			if(mst->to==0) mst->to=offset;
			offset=header->link;
			continue;
		}
		// This file is valid(not deleted, reclaim may start)
		else
		{
			// If this object is first object, just skip it
			if(offset==device_info.j4fs_offset)
			{
				offset=header->link;
				continue;
			}

			// If invalid object is not scanned yet, there are no place to copy this valid object
			if(mst->to==0)
			{
				offset=header->link;
				continue;
			}

			/**************************************************************************
			 * Start to copy this valid object to invalid area(to offset)
			 **************************************************************************/
			mst->from=offset;
			offset += J4FS_BASIC_UNIT_SIZE;	// header
			offset += header->length;	// data
			offset = (offset+J4FS_BASIC_UNIT_SIZE-1)/J4FS_BASIC_UNIT_SIZE*J4FS_BASIC_UNIT_SIZE;	// J4FS_BASIC_UNIT_SIZE align. offset indicate 'end of this object and beginning of next object'
			mst->end=offset;
			mst->copyed=0;
			mst->status=J4FS_RECLAIM_MOVING_DATA_STEP_1;

			if(header->link==0xffffffff) mst->status|=J4FS_RECLAIM_LAST_OBJECT;

			J4FS_POR(0x1,("%s %d: Power-off point-1\n",__FUNCTION__,__LINE__),2000);

			// write mst reflected new status(from,end)
			ret = FlashDevWrite(&device_info, 0, J4FS_BASIC_UNIT_SIZE, buf_mst);
			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
		   		goto error1;
			}

moving_data_step_1:

			J4FS_POR(0x2,("%s %d: Power-off point-2\n",__FUNCTION__,__LINE__),2000);

			// copy valid data('from' offset) to invalid area('to' offset)
			while(mst->from < mst->end)
			{
				J4FS_POR(0x4,("%s %d: Power-off point-4\n",__FUNCTION__,__LINE__),2000);

				// read valid data
				ret = FlashDevRead(&device_info, mst->from, J4FS_BASIC_UNIT_SIZE, buf_data);
				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
			   		goto error1;
				}

				J4FS_POR(0x8,("%s %d: Power-off point-8\n",__FUNCTION__,__LINE__),2000);

				// write valid data
				ret = FlashDevWrite(&device_info, mst->to, J4FS_BASIC_UNIT_SIZE, buf_data);
				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
			   		goto error1;
				}

				J4FS_POR(0x10,("%s %d: Power-off point-10\n",__FUNCTION__,__LINE__),2000);

				// write updated MST status(from,to)
				mst->from+=J4FS_BASIC_UNIT_SIZE;
				mst->to+=J4FS_BASIC_UNIT_SIZE;
				mst->copyed+=J4FS_BASIC_UNIT_SIZE;
				ret = FlashDevWrite(&device_info, 0, J4FS_BASIC_UNIT_SIZE, buf_mst);
				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
			   		goto error1;
				}

				J4FS_POR(0x20,("%s %d: Power-off point-20\n",__FUNCTION__,__LINE__),2000);

			}

			J4FS_POR(0x40,("%s %d: Power-off point-40\n",__FUNCTION__,__LINE__),2000);

			// write updated MST status(offset,offset_number,from,to)
			mst->offset[mst->offset_number]=mst->to-mst->copyed;
			mst->offset_number++;
			mst->status= (mst->status&J4FS_RECLAIM_LAST_OBJECT)|J4FS_RECLAIM_MOVING_DATA_STEP_2;
			ret = FlashDevWrite(&device_info, 0, J4FS_BASIC_UNIT_SIZE, buf_mst);
			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
		   		goto error1;
			}

moving_data_step_2:
			if(mst->status&J4FS_RECLAIM_LAST_OBJECT) offset=0xffffffff;
			else offset=mst->end;

			J4FS_POR(0x80,("%s %d: Power-off point-80\n",__FUNCTION__,__LINE__),2000);

			/**************************************************************************
			 * End of copying this valid object to invalid area(to offset)
			 **************************************************************************/
		}
	}

	J4FS_POR(0x100,("%s %d: Power-off point-100\n",__FUNCTION__,__LINE__),2000);

	if(mst->offset_number==0) goto reclaim_done;

	// Adjust j4fs_header.link of valid object
	// write updated MST status(status)
	mst->status=J4FS_RECLAIM_UPDATE_LINK;
	ret = FlashDevWrite(&device_info, 0, J4FS_BASIC_UNIT_SIZE, buf_mst);
	if (error(ret)) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
   		goto error1;
	}

update_link:

	J4FS_POR(0x200,("%s %d: Power-off point-200\n",__FUNCTION__,__LINE__),2000);

	// Adjust j4fs_header.link of valid objects excluding last object
	if(mst->offset_number>=2)
	{
		for(i=0;i<mst->offset_number-1;i++)
		{
			J4FS_POR(0x400,("%s %d: Power-off point-400\n",__FUNCTION__,__LINE__),2000);

			// read j4fs_header
			ret = FlashDevRead(&device_info, mst->offset[i], J4FS_BASIC_UNIT_SIZE, buf_header);
			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
		   		goto error1;
			}

			J4FS_POR(0x800,("%s %d: Power-off point-800\n",__FUNCTION__,__LINE__),2000);

			header->link=mst->offset[i+1];

			// write j4fs_header
			ret = FlashDevWrite(&device_info, mst->offset[i], J4FS_BASIC_UNIT_SIZE, buf_header);
			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
		   		goto error1;
			}

			J4FS_POR(0x1000,("%s %d: Power-off point-1000\n",__FUNCTION__,__LINE__),2000);

		}
	}

	J4FS_POR(0x2000,("%s %d: Power-off point-2000\n",__FUNCTION__,__LINE__),2000);

	// Adjust j4fs_header.link of last valid object
	if(mst->offset_number>0)
	{
		J4FS_POR(0x4000,("%s %d: Power-off point-4000\n",__FUNCTION__,__LINE__),2000);

		// read j4fs_header
		ret = FlashDevRead(&device_info, mst->offset[mst->offset_number-1], J4FS_BASIC_UNIT_SIZE, buf_header);
		if (error(ret)) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
	   		goto error1;
		}

		J4FS_POR(0x8000,("%s %d: Power-off point-8000\n",__FUNCTION__,__LINE__),2000);

		header->link=0xffffffff;

		// write j4fs_header
		ret = FlashDevWrite(&device_info, mst->offset[mst->offset_number-1], J4FS_BASIC_UNIT_SIZE, buf_header);
		if (error(ret)) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
	   		goto error1;
		}

		J4FS_POR(0x10000,("%s %d: Power-off point-10000\n",__FUNCTION__,__LINE__),2000);

		first_unused_area_offset = mst->offset[mst->offset_number-1] + J4FS_BASIC_UNIT_SIZE + header->length;
		first_unused_area_offset = (first_unused_area_offset + J4FS_BASIC_UNIT_SIZE-1)/J4FS_BASIC_UNIT_SIZE*J4FS_BASIC_UNIT_SIZE;

	}

	J4FS_POR(0x20000,("%s %d: Power-off point-20000\n",__FUNCTION__,__LINE__),2000);

	// write 'Reclaim Done'
	rw_start=mst->rw_start;
	memset(mst,0x0,J4FS_BASIC_UNIT_SIZE);
	mst->magic=J4FS_MAGIC;
	mst->status=J4FS_RECLAIM_DONE;
	mst->rw_start=rw_start;

#ifdef J4FS_TRANSACTION_LOGGING
	// setting transaction variable
	memset(transaction,0xff,J4FS_TRANSACTION_SIZE);
	transaction->magic=J4FS_MAGIC;
	transaction->sequence=j4fs_next_sequence++;
	transaction->opcode=J4FS_RECLAIM_STEP2;

	ret = FlashDevWrite(&device_info, j4fs_transaction_next_offset, J4FS_TRANSACTION_SIZE, (BYTE *)transaction);

	if (error(ret)) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
		goto error1;
	}

	j4fs_transaction_next_offset+=J4FS_TRANSACTION_SIZE;
	if(j4fs_transaction_next_offset>=device_info.j4fs_device_end) j4fs_transaction_next_offset=device_info.j4fs_end;
#endif

	ret = FlashDevWrite(&device_info, 0, J4FS_BASIC_UNIT_SIZE, buf_mst);
	if (error(ret)) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
   		goto error1;
	}

#ifdef J4FS_RECLAIM_RESET_UNUSED_SPACE
	// Set 'not used space' to 0xff. This is not mandatory
	if(first_unused_area_offset!=0xffffffff)
	{
		memset(buf_data,0xff,J4FS_BASIC_UNIT_SIZE);
		for(i=0; i<10*J4FS_BASIC_UNIT_SIZE; i+=J4FS_BASIC_UNIT_SIZE)
		{
			J4FS_T(J4FS_TRACE_FSD_RECLAIM,("%s %d: (first_unused_area_offset=0x%08x,i=0x%08x,j4fs_end=0x%08x)\n",__FUNCTION__,__LINE__,first_unused_area_offset,i,device_info.j4fs_end));
			ret = FlashDevWrite(&device_info, first_unused_area_offset+i, J4FS_BASIC_UNIT_SIZE, buf_data);
			if (error(ret)) {
				J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
		   		goto error1;
			}
		}
	}
#endif

reclaim_done:
	j4fs_rw_start=mst->rw_start;

	// Handle the exception
	if(!j4fs_rw_start ||(j4fs_rw_start>= device_info.j4fs_end))
	{
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: j4fs_rw_start is set to default value(128KB)\n",__FUNCTION__,__LINE__));
		j4fs_rw_start=device_info.j4fs_offset;
	}

	if(!ro_j4fs_header_count) fsd_read_ro_header();

#ifdef __KERNEL__
	kfree(buf_mst);
	kfree(buf_header);
	kfree(buf_data);
#ifdef J4FS_TRANSACTION_LOGGING
	kfree(transaction);
#endif
#endif

	J4FS_T(J4FS_TRACE_FSD_RECLAIM,("\n%s %d: Reclaim Done\n",__FUNCTION__,__LINE__));
	fsd_print_meta_data();

	return J4FS_SUCCESS;

error1:
	J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));

	// Handle the exception
	if(!j4fs_rw_start ||(j4fs_rw_start>= device_info.j4fs_end))
	{
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: j4fs_rw_start is set to default value(128KB)\n",__FUNCTION__,__LINE__));
		j4fs_rw_start=device_info.j4fs_offset;
	}

	if(!ro_j4fs_header_count) fsd_read_ro_header();
	fsd_print_meta_data();

#ifdef __KERNEL__
	kfree(buf_mst);
	kfree(buf_header);
	kfree(buf_data);
#ifdef J4FS_TRANSACTION_LOGGING
	kfree(transaction);
#endif
#endif
	return J4FS_FAIL;
}

extern DWORD valid_offset[128][2];
// invalidate old valid files
int fsd_mark_invalid()
{
	DWORD offset;
	j4fs_header *header;
	int index=0;
	int i,j;
	j4fs_mst *mst;
	int ret=-1;

#ifdef __KERNEL__
	BYTE *buf;
	buf=kmalloc(J4FS_BASIC_UNIT_SIZE,GFP_NOFS);
#else
	BYTE buf[J4FS_BASIC_UNIT_SIZE];
#endif

	mst=(j4fs_mst *)buf;

	// read mst
	ret = FlashDevRead(&device_info, 0, J4FS_BASIC_UNIT_SIZE, buf);
	if (error(ret)) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
   		goto error1;
	}

	if(mst->magic==J4FS_MAGIC)
	{
		// the start address of the RW area of the device (partition)
		offset=mst->rw_start;

		if(!offset ||(offset>= device_info.j4fs_end))
		{
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error! rw_start is invalid(j4fs_rw_start=0x%08x, j4fs_end=0x%08x)\n", __FUNCTION__, __LINE__, mst->rw_start, device_info.j4fs_end));
			j4fs_panic("rw_start is invalid");
			goto error1;
		}
	}
	else
	{
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error! MST is invalid\n", __FUNCTION__, __LINE__));
		j4fs_panic("MST is invalid");
		goto error1;
	}

	// trace all RW object header
	while(offset!=0xffffffff)
	{
		// read j4fs_header
		ret = FlashDevRead(&device_info, offset, J4FS_BASIC_UNIT_SIZE, buf);
		if (error(ret)) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
	   		goto error1;
		}
		header=(j4fs_header *)buf;

		//This j4fs_header cannot be interpreted. It means there are no RW files in this partition(this can happen and this is a normal case) or
		//this j4fs partition is crashed(this should not happen).
		if(header->type!=J4FS_FILE_TYPE)
		{
			// There are no RW files in this partition or this first j4fs_header is crashed. Before we write data of new file, user of j4fs should write j4fs_header of new file.
			// So, this case should not happen and/or should be repaired.
			if(offset==mst->rw_start) {
				j4fs_panic("There are no RW files in this partition or this first RW j4fs_header is crashed. Before we write data of new file, user of j4fs should write j4fs_header of new file. So, this case should not happen and/or should be repaired..");
				goto error1;
			}

			// This j4fs partition is crashed by some abnormal cause. This should not happen and should be repaired.
			j4fs_panic("this j4fs partition is crashed by some abnormal cause.  This should not happen and should be repaired.");
			goto error1;
		}

		// This file was deleted, so read next j4fs_header.
		if((header->flags&0x1)!=((header->flags&0x2)>>1))
		{
			offset=header->link;
			continue;
		}

		// This file is valid.
		valid_offset[index][0]=header->id;
		valid_offset[index][1]=offset;
		index++;
		offset=header->link;
	}

	J4FS_T(J4FS_TRACE_FSD_PRINT_META_DATA,("%s %d: index=%d\n",__FUNCTION__,__LINE__,index));

	// Invalidate old files. we exclude last object header because last object header can't decide invalidation of old files
	for(i=0;i<index-2;i++)
	{
		for(j=i+1;j<index-1;j++)
		{
			// If inode number is same, invalidate old object
			if(valid_offset[i][0]==valid_offset[j][0])
			{
				//J4FS_T(J4FS_TRACE_FSD_PRINT_META_DATA,("%s %d\n",__FUNCTION__,__LINE__));

				// read j4fs_header
				ret = FlashDevRead(&device_info, valid_offset[i][1], J4FS_BASIC_UNIT_SIZE, buf);
				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
			   		goto error1;
				}
				header=(j4fs_header *)buf;

				//This j4fs_header cannot be interpreted. It means there are no RW files in this partition(this can happen and this is a normal case) or
				//this j4fs partition is crashed(this should not happen).
				if(header->type!=J4FS_FILE_TYPE)
				{
					// There are no RW files in this partition or this first j4fs_header is crashed. Before we write data of new file, user of j4fs should write j4fs_header of new file.
					// So, this case should not happen and/or should be repaired.
					if(offset==mst->rw_start) {
						j4fs_panic("There are no RW files in this partition or this first RW j4fs_header is crashed. Before we write data of new file, user of j4fs should write j4fs_header of new file. So, this case should not happen and/or should be repaired..");
						goto error1;
					}

					// This j4fs partition is crashed by some abnormal cause. This should not happen and should be repaired.
					j4fs_panic("this j4fs partition is crashed by some abnormal cause.  This should not happen and should be repaired.");
					goto error1;
				}

				// this file will be deleted
				header->flags=0x1;

				ret = FlashDevWrite(&device_info, valid_offset[i][1], J4FS_BASIC_UNIT_SIZE, buf);
				if (error(ret)) {
					J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
			   		goto error1;
				}
				break;
			}
		}
	}

#ifdef __KERNEL__
	kfree(buf);
#endif

	return J4FS_SUCCESS;

error1:
#ifdef __KERNEL__
	kfree(buf);
#endif
	return J4FS_FAIL;
}


int fsd_special(j4fs_ctrl *ctl)
{
	return FlashDevSpecial(&device_info,ctl->scmd);
}

// Read j4fs_header list of RO area
int fsd_read_ro_header(void)
{
	DWORD offset;
	j4fs_header *header;
	int i;
	int ret=-1;

#ifdef __KERNEL__
	BYTE *buf;
	buf=kmalloc(J4FS_BASIC_UNIT_SIZE,GFP_NOFS);
#else
	BYTE buf[J4FS_BASIC_UNIT_SIZE];
#endif

	ro_j4fs_header_count=0;

	// the start address of the device (partition)
	offset=	device_info.j4fs_offset;

	// scanning all j4fs_header of RO area.
	while(offset<j4fs_rw_start)
	{
		// read j4fs_header
		ret = FlashDevRead(&device_info, offset, J4FS_BASIC_UNIT_SIZE, buf);
		if (error(ret)) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
			goto error1;
		}
		header=(j4fs_header *)buf;

		//This j4fs_header cannot be interpreted.
		if(header->type!=J4FS_FILE_TYPE)
		{
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: ERROR! RO file cannot be interpreted. (offset=%d)\n",__FUNCTION__,__LINE__,offset));
			j4fs_panic("This j4fs_header cannot be interpreted. So this j4fs partition is crashed by some abnormal cause.  This should not happen and should be repaired.");
			goto error1;
		}

		// This file was deleted, so read next j4fs_header.
		if((header->flags&0x1)!=((header->flags&0x2)>>1))
		{
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: ERROR! RO file was deleted. (offset,ino,filename)=(%d,%d,%s)\n",__FUNCTION__,__LINE__,offset,header->id,header->filename));
			offset=header->link;
			continue;
		}

		if(ro_j4fs_header_count>=J4FS_MAX_RO_FILES_NUMBER)
		{
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: ERROR! Too many ro files\n",__FUNCTION__,__LINE__));

			j4fs_traceMask |= J4FS_TRACE_FSD_PRINT_META_DATA;
			fsd_print_meta_data();
			j4fs_traceMask &= ~J4FS_TRACE_FSD_PRINT_META_DATA;
			goto error1;
		}

		memcpy(ro_j4fs_header+ro_j4fs_header_count, header, sizeof(j4fs_header));
		ro_j4fs_header_count++;

		offset=header->link;
	}

	J4FS_T(J4FS_TRACE_FSD_PRINT_META_DATA,("\n====================== %s %d ================================\n", __FUNCTION__,__LINE__));

	for(i=0;i<ro_j4fs_header_count;i++)
	{
		J4FS_T(J4FS_TRACE_FSD_PRINT_META_DATA,("%d : (link,type,flags,id,length,filename)=(0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,%s)\n",(i+1),ro_j4fs_header[i].link, ro_j4fs_header[i].type, ro_j4fs_header[i].flags, ro_j4fs_header[i].id, ro_j4fs_header[i].length, ro_j4fs_header[i].filename));
	}

	J4FS_T(J4FS_TRACE_FSD_PRINT_META_DATA,("\n====================== %s %d ================================\n", __FUNCTION__,__LINE__));
#ifdef __KERNEL__
	kfree(buf);
#endif
	return J4FS_SUCCESS;

error1:
#ifdef __KERNEL__
	kfree(buf);
#endif
	return J4FS_FAIL;

}

#ifdef J4FS_TRANSACTION_LOGGING
int fsd_initialize_transaction()
{
	int i, ret;
	DWORD sequence=0,offset=0xffffffff;
	j4fs_transaction *transaction;

#ifdef __KERNEL__
	BYTE *buf;
	buf=kmalloc(J4FS_TRANSACTION_SIZE,GFP_NOFS);
#else
	BYTE buf[J4FS_TRANSACTION_SIZE];
#endif

	transaction=(j4fs_transaction *)buf;

	for(i=device_info.j4fs_end+1; i<device_info.j4fs_device_end; i+=J4FS_TRANSACTION_SIZE)
	{
		ret = FlashDevRead(&device_info, i, J4FS_TRANSACTION_SIZE, buf);
		if (error(ret)) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(ret=0x%08x)\n",__FUNCTION__,__LINE__,ret));
	   		goto error1;
		}

		if(transaction->magic!=J4FS_MAGIC) continue;
		if(sequence<transaction->sequence)
		{
			sequence=transaction->sequence;
			offset=i;
		}
	}

	// there are no file system transaction
	if(offset==0xffffffff)
	{
		j4fs_next_sequence=1;
		j4fs_transaction_next_offset=device_info.j4fs_end+1;
	}
	else
	{
		j4fs_next_sequence=sequence+1;
		j4fs_transaction_next_offset=offset+512;

		if(j4fs_transaction_next_offset>=device_info.j4fs_device_end) j4fs_transaction_next_offset=device_info.j4fs_end+1;
	}

#ifdef __KERNEL__
	kfree(buf);
#endif
	return J4FS_SUCCESS;

error1:
#ifdef __KERNEL__
	kfree(buf);
#endif
	return J4FS_FAIL;
}
#endif

int fsd_panic()
{
	j4fs_mst *mst;
	int ret=-1;

#ifdef __KERNEL__
	BYTE *buf;
	buf=kmalloc(J4FS_BASIC_UNIT_SIZE,GFP_NOFS);
#else
	BYTE buf[J4FS_BASIC_UNIT_SIZE];
#endif

	j4fs_panic=1;

	// Marking j4fs panic by writing J4FS_PANIC to mst->status
	ret = FlashDevRead(&device_info, 0, J4FS_BASIC_UNIT_SIZE, buf);
	if (error(ret)) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error\n",__FUNCTION__,__LINE__));
   		goto error1;
	}

	mst=(j4fs_mst *)buf;
	mst->status|=J4FS_PANIC;

	ret = FlashDevWrite(&device_info, 0, J4FS_BASIC_UNIT_SIZE, buf);
	if (error(ret)) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
   		goto error1;
	}

#ifdef __KERNEL__
	kfree(buf);
#endif
	return J4FS_SUCCESS;

error1:
#ifdef __KERNEL__
	kfree(buf);
#endif
	return J4FS_FAIL;
}

int fsd_print_meta_data()
{
	DWORD offset;
	j4fs_header *header;
	j4fs_mst *mst;
	int i;
	int ret=-1;

#ifdef __KERNEL__
	BYTE *buf;
	buf=kmalloc(4096,GFP_NOFS);
#else
	BYTE buf[4096];
#endif

	if(!(j4fs_traceMask|J4FS_TRACE_FSD_PRINT_META_DATA)) return 0;

	// print MST
	ret = FlashDevRead(&device_info, 0, J4FS_BASIC_UNIT_SIZE, buf);
	if (error(ret)) {
		J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error\n",__FUNCTION__,__LINE__));
   		goto error1;
	}

	J4FS_T(J4FS_TRACE_FSD_PRINT_META_DATA,("====================================================================\n"));

	mst=(j4fs_mst *)buf;
	J4FS_T(J4FS_TRACE_FSD_PRINT_META_DATA,("(magic,from,to,end,offset_number,status,rw_start)=(0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x)\n",
		mst->magic,mst->from,mst->to,mst->end,mst->offset_number,mst->status,mst->rw_start));

	if(mst->magic==J4FS_MAGIC && mst->offset_number)
	{
		J4FS_T(J4FS_TRACE_FSD_PRINT_META_DATA,("offset: "));
		for(i=0;i<mst->offset_number;i++)
			J4FS_T(J4FS_TRACE_FSD_PRINT_META_DATA,("0x%x ", mst->offset[i]));
		J4FS_T(J4FS_TRACE_FSD_PRINT_META_DATA,("\n"));
	}

	// the start address of the device (partition)
	offset=	device_info.j4fs_offset;

	i=0;
	while(1)
	{
		// read j4fs_header
		ret = FlashDevRead(&device_info, offset, J4FS_BASIC_UNIT_SIZE, buf);
		if (error(ret)) {
			J4FS_T(J4FS_TRACE_ALWAYS,("%s %d: Error(nErr=0x%08x)\n",__FUNCTION__,__LINE__,ret));
	   		goto error1;
		}
		header=(j4fs_header *)buf;

		//This j4fs_header cannot be interpreted.
		if(header->type!=J4FS_FILE_TYPE) break;

		J4FS_T(J4FS_TRACE_FSD_PRINT_META_DATA,("%d : (link,type,flags,id,length,filename)=(0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,%s)\n",++i,header->link,header->type,header->flags,header->id,header->length,header->filename));

		// this is last enry in the device
		if(header->link==0xffffffff)
		{
			break;
		}
		else
		{
			offset += J4FS_BASIC_UNIT_SIZE;	// header
			offset += header->length;	// data
			offset = (offset+J4FS_BASIC_UNIT_SIZE-1)/J4FS_BASIC_UNIT_SIZE*J4FS_BASIC_UNIT_SIZE;	// 4096 align
			if(mst->from && offset>=mst->to) offset=mst->from;	// In case of power-off when reclaim(mst-from>0), skip from 'to offset' to 'from offset'
		}
	}

	J4FS_T(J4FS_TRACE_FSD_PRINT_META_DATA,("====================================================================\n"));

#ifdef __KERNEL__
	kfree(buf);
#endif
	return J4FS_SUCCESS;

error1:
#ifdef __KERNEL__
	kfree(buf);
#endif
	return J4FS_FAIL;
}
