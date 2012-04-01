/*
 * Copyright (C) 2008, 2009,  Samsung Electronics Co. Ltd. All Rights Reserved.
 *       Written by Linux Lab, MITs Development Team, Mobile Communication Division.
 */

/*
 * J4FS
 *
 * Jong Jang Jintae Jongmin File System is based on LFS(Linear File Store)
 *
 *
 * 2009.02 - First editing by SungHwan.yun <sunghwan.yun@samsung.com> @LDK@
 *
 * 2009.03 - Currently managed by  SungHwan.yun <sunghwan.yun@samsung.com> @LDK@
 *
 * 2009.12 - Version 1.0 Released by SungHwan.yun <sunghwan.yun@samsung.com> @LDK@
 *
 */

#ifdef __KERNEL__
#include <linux/version.h>
#include <linux/types.h>
#include <asm/types.h>
#include <linux/spinlock.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/buffer_head.h>

#ifndef BYTE
#define BYTE __u8
#endif

#ifndef WORD
#define WORD __u16
#endif

#ifndef HWORD
#define HWORD __u16
#endif

#ifndef DWORD
#define DWORD __u32
#endif

#define PRINT printk

#else

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef WORD
#define WORD unsigned int
#endif

#ifndef HWORD
#define HWORD unsigned int
#endif

#ifndef DWORD
#define DWORD unsigned int
#endif

#define PRINT printf

#endif

/**********************************************************
 * This is porting values
 **********************************************************/
#define PHYSICAL_PAGE_SIZE		2048		// 4KB
#define PHYSICAL_BLOCK_SIZE	PHYSICAL_PAGE_SIZE*64		// 256KB
#define J4FS_PARTITION_ID		21
#undef J4FS_USE_XSR							// NO XSR
// J4FS for moviNAND merged from ROSSI
#undef J4FS_USE_FSR						// FSR
#define J4FS_USE_MOVI

#if defined(J4FS_USE_MOVI)
#define J4FS_BLOCK_COUNT	20
#endif
// J4FS for moviNAND merged from ROSSI

/*
 * J4FS Version(J4FS_1.0.0p0_b0)
 */
#define J4FS_VER_MAJOR			1
#define J4FS_VER_MINOR1			0
#define J4FS_VER_MINOR2			0
#define J4FS_VER_PATCHLEVEL		0
#define J4FS_BUILD_NUMBER		0


/*
 * file header(j4fs_header) and data block and media status table(j4fs_mst) size
 */
#define J4FS_BASIC_UNIT_SIZE			2048
#define J4FS_BASIC_UNIT_SIZE_BITS		11

/*
 * File name length
 */
#define J4FS_NAME_LEN 128

/*
 * Max RO file number
 */
#define J4FS_MAX_RO_FILES_NUMBER 200

/*
  * Max file number in J4FS
 */
#define J4FS_MAX_FILE_NUM	256

/*
 * Special inode numbers
 */
#define J4FS_BAD_INO		 1	/* Bad blocks inode */
#define J4FS_ROOT_INO		 2	/* Root inode */
#define J4FS_BOOT_LOADER_INO	 5	/* Boot loader inode */
#define J4FS_UNDEL_DIR_INO	 6	/* Undelete directory inode */
#define J4FS_FIRST_INO	11 /* First non-reserved inode */

/*
  * Reclaim status
 */
#define J4FS_RECLAIM_DONE							0x01230000	// This value should not be zero becaust of POR
#define J4FS_RECLAIM_MOVING_DATA_STEP_1		0x00000001
#define J4FS_RECLAIM_MOVING_DATA_STEP_2		0x00000002
#define J4FS_RECLAIM_UPDATE_LINK					0x00000004
#define J4FS_RECLAIM_LAST_OBJECT					0x10000000
#define J4FS_PANIC										0x00005a00
#define J4FS_PANIC_MASK								0x0000ff00

// Debug
#define J4FS_TRACE_FSD					0x00000001
#define J4FS_TRACE_FS						0x00000002
#define J4FS_TRACE_FS_READ				0x00000004
#define J4FS_TRACE_LOCK					0x00000008
#define J4FS_TRACE_FSD_RECLAIM		0x00000010
#define J4FS_TRACE_FSD_PRINT_META_DATA		0x00000020

#define J4FS_TRACE_ALWAYS		0xF0000000

/*
 * Define return value
 */
#define J4FS_SUCCESS			0x0
#define J4FS_RETRY_WRITE		0x20000000
#define J4FS_FAIL					0x40000000
#define J4FS_NO_FILE			0x40001000

/*
 * opcode : j4fs transaction for j4fs crash debugging
 */
#define J4FS_LAST_OBJECT_CREATE_STEP1							0x1		// 1,4,10
#define J4FS_LAST_OBJECT_CREATE_STEP2							0x2		// 1,4,10
#define J4FS_LAST_OBJECT_WRITE_EXTEND1						0x10		// 2,3,5,6
#define J4FS_LAST_OBJECT_WRITE_EXTEND2						0x20		// 8
#define J4FS_NOLAST_OBJECT_WRITE_EXTEND_STEP1			0x100		// 9,11,12,13
#define J4FS_NOLAST_OBJECT_WRITE_EXTEND_STEP2			0x110	// 9,11,12,13
#define J4FS_NOLAST_OBJECT_WRITE_EXTEND_STEP3			0x120	// 9,11,12,13
#define J4FS_RECLAIM_STEP1											0x1000	// reclaim
#define J4FS_RECLAIM_STEP2											0x2000	// reclaim

#define J4FS_RECLAIM_RESET_UNUSED_SPACE
#define J4FS_TRANSACTION_LOGGING

#define T(mask, p) do { if ((mask) & (j4fs_traceMask | J4FS_TRACE_ALWAYS)) TOUT(p); } while (0)
#define POR(mask, p, q) do { if (((mask) & (j4fs_PORMask))&&!(--j4fs_PORCount)) {TOUT(p); while(1); }} while (0)

#define error(ret)	(ret>=J4FS_FAIL)

#ifdef __KERNEL__
#define j4fs_panic(str)		\
do {							\
	T(J4FS_TRACE_ALWAYS,("%s %d: "str"\n",__FUNCTION__,__LINE__));	\
	fsd_panic();		\
	dump_stack();	\
} while (0)
#else
#define j4fs_panic(str)		\
do {							\
	T(J4FS_TRACE_ALWAYS,("%s %d: "str"\n",__FUNCTION__,__LINE__));	\
	fsd_panic();	\
} while (0)
#endif

#ifdef __KERNEL__
#define j4fs_dump(str)		\
do {							\
	T(J4FS_TRACE_ALWAYS,("%s %d: "str"\n",__FUNCTION__,__LINE__));	\
	dump_stack();	\
} while (0)
#else
#define j4fs_dump(str)		\
do {							\
	T(J4FS_TRACE_ALWAYS,("%s %d: "str"\n",__FUNCTION__,__LINE__));	\
	while(1);	\
} while (0)
#endif


#define j4fs_check_partition_range(offset)		\
do {		\
	if(offset + J4FS_BASIC_UNIT_SIZE > device_info.j4fs_end) {	\
		T(J4FS_TRACE_ALWAYS,("%s %d: offset overflow(offset=0x%08x, j4fs_end=0x%08x)\n", __FUNCTION__, __LINE__, offset, device_info.j4fs_end));	\
		j4fs_panic("offset overflow!!");	\
		goto error1;	\
	}	\
} while(0)



#ifdef __KERNEL__
/*
 * Flash data structure of the super block
 */
struct j4fs_super_block {
	unsigned long pad;
};

/*
 * Memory data structure of the super block
 */
struct j4fs_sb_info {
	struct j4fs_super_block * s_es;	/* Pointer to the super block in the buffer */
	unsigned long	s_log_block_size;	/* Block size(0:1024, 1:2048, 3:4096)*/
	int s_first_ino;
};

/*
 * Flash data structure of the inode
 */
struct j4fs_inode {
	__le32 i_link;
	__le32 i_size;
	__le32 i_type;
	__le32 i_offset;
	__le32 i_flags;
	__le32 i_stroff;
	__le32 i_id;
	__le32	i_length;
	__u8 i_filename[J4FS_NAME_LEN];
};

/*
 * Memory data structure of the inode
 */
struct j4fs_inode_info {
	__le32 i_link;
	__le32 i_type;
	__le32 i_flags;
	__le32 i_id;
	__le32	i_length;
	__u8 i_filename[J4FS_NAME_LEN];
	rwlock_t i_meta_lock;
	struct inode	vfs_inode;
};

#endif


#define J4FS_INIT				1
#define J4FS_GET_INFO		2
#define J4FS_EXIT			4

#ifndef __KERNEL__
#define J4FS_FORMAT		3
#endif

#define J4FS_FILE_TYPE		0x12345678
#define J4FS_MAGIC			0x87654321

/*
  * device : This field indicates the device(partition) number to be acted upon. Device can also be thought of as a partition. This field is STL partition id.
  * status : This field provides a mechanism to pass a detailed failure back to the application. Typical functions will place a detailed error in this field
  *               and return SUCCESS or FAILURE back to the calling function.
  * buffer  : This field provides a pointer to a memory buffer to translate data either to or from a function
  * count  : This field indicates the number of bytes transferred. This field is usually used on write operations
  * actual  : This field indicates whether or not a read or write is possible to a position by comparing the requested source file size with the actual size
  *              of the target position.
  * scmd   : This field provides a subcommand for possible use in the future
  * type    : PCMCIA requires that LFS headers contain a stamp indicating the type of the header. For our implementation, this field has been assigned
  *              ZERO (0), indicating the 32 byte header above.
  * id        : This value is unique to each file object. This unique id field is to be used as a file identifier
  * aux     : This field does not get initialized in the FlashDevMount function. It provides a mechanism to pass extra information to/from the low-level
  *              function set
  * index   : file offset to read or write
 */
typedef struct {
	DWORD device;
	DWORD status;
	BYTE *buffer;
	DWORD count;
	DWORD actual;
	DWORD scmd;
	DWORD type;
	DWORD id;
	DWORD aux;
	DWORD index;
} j4fs_ctrl;


/*
  * we assume j4fs_header is aligned with page size
  *
  * link     : This field contains the offset from the start of this header to the next LFS header in the device(partition). If each bit in this field is equal to bit D0
  *             of the flags field, this is the last entry in the device.
  * size    : This is the actual size of the LFS header. [Obsoleted]
  * type   : PCMCIA requires that LFS headers contain a stamp indicating the type of the header. For our implementation, this field has been assigned ZERO
  *             (0), indicating the 32-byte header above. This type field is assigned to this header to distinguish it from other headers that may exist
  *             in the device. This prevents the FSD from reading a header it cannot interpret.
  * offset : This field indicates how far from the start of this header into the entry the file data begins. This allows LFS implementations that use extended
  *             headers to be compatible with drivers that can't read the extended header.[Obsoleted]
  * flags   : This is a bit-mapped flags field. Bit D0 indicates the nature of the flash (1 erase or 0 erase). Bit D1 indicates whether or not this file entry is
  *             valid or deleted: if D1 matches D0 the file is valid, if D1 differs, than that file is deleted
  * stroff  : This field points to the extended header associated with this file. The actual location of the filename string is determined by adding the value
  *             in this field to the address of the LFS_HEADER. If this field is zero, there is no filename.[Obsoleted]
  * id       : This value is unique to each file object. This unique id field is to be used as a file identifier
  * length : file data length
  * filename : filename
*/
typedef struct {
	DWORD link;
	DWORD size;
	DWORD type;
	DWORD offset;
	DWORD flags;
	DWORD stroff;
	DWORD id;
	DWORD length;
	BYTE filename[J4FS_NAME_LEN];
} j4fs_header;


/*
  * device  : This field indicates the device (partition) number to be acted upon. Device can also be thought of as a partition. This field is STL partition id.
  * status  : This field provides a mechanism to pass a detailed failure back to the application. Typical functions will place a detailed error in this field and
                   return SUCCESS or FAILURE back to the calling function
  * blocksize : This field should be initialized by the FlashDevMount function and provides a mechanism to allow multiple devices (partitions) with
  *                  different Intel components (which may have different block sizes). This field should reflect the size of media that will be erased when
  *                  one block is erased in Bytes.
  * pagesize  : This field indicates the size of page in Bytes.
  * numberblocks : This field indicates the number of blocks in the entire media. If their are two flash cards, each would be considered its own media.
  *                         If the media is an RFA, each RFA is considered its own media
  * j4fs_offset : This field indicates the beginning address of data area of the device (partition) in Bytes. This address follows the
  *                     Media Status Table(MST) and MST starts from offset 0.
  * j4fs_size : This value indicates the size of the device (partition) in Bytes.
  * j4fs_end : This field indicates the address of the end of the current device(partition) in Bytes. The device(partition) starts from offset 0.
  * aux : This field does not get initialized in the FlashDevMount function. It provides a mechanism to pass extra information to/from the low-level function set
  */
typedef struct {
	DWORD device;
	DWORD status;
	DWORD blocksize;
	DWORD pagesize;
	DWORD numberblocks;
	DWORD j4fs_offset;
	DWORD j4fs_size;
	DWORD j4fs_end;
	DWORD j4fs_device_end;
	DWORD aux;

#ifdef __KERNEL__
	struct semaphore grossLock;	/* Gross locking semaphore */
#endif
} j4fs_device_info;


/*
  * J4FS Media Status Table(MST)
  * This structure is used for reclaim including POR. The MST will reserve one block and will be used during initialization and reclaim.
  * This MST is intended to assist in making the J4FS more robust. It provides memory to store status updates when reclaim is occurring,
  * so that the system may recover form an unexpected power-off. If (from,to,end) is all 0, no reclaim is needed or reclaim is done.(Initial state)
       If (from,to,end) is not 0, reclaim is in progress and should reclaim-restart from 'from,to member'
  * magic : J4FS_MAGIC
  * from , to(Bytes) : copy from 'from offset' to 'to offset' (4096Bytes data).
  * end(Bytes) : end offset of 'from offset'.
  * offset(Bytes) : beginning offsets of new arranged valid files excluding invalid files
  * offset_number : number of offset array
  */
typedef struct {
	DWORD magic;
	DWORD from;
	DWORD to;
	DWORD end;
	DWORD copyed;
	DWORD offset[J4FS_MAX_FILE_NUM];
	DWORD offset_number;
	DWORD status;
	DWORD rw_start;
} j4fs_mst;

#ifdef J4FS_TRANSACTION_LOGGING
/*
  * transaction structure for j4fs crash debugging. size should be 512B.
  */
typedef struct {
	DWORD offset;
	DWORD opcode;
	DWORD sequence;
	DWORD magic;

	DWORD ino;
	DWORD index;
	DWORD count;
	DWORD reserved1;

	DWORD b_link;
	DWORD b_length;
	DWORD a_link;
	DWORD a_length;

	BYTE filename[J4FS_NAME_LEN];
	BYTE reserved2[336];
} j4fs_transaction;

#define J4FS_TRANSACTION_SIZE	512
#endif

extern int j4fs_close(void);

#ifdef __KERNEL__
extern ino_t j4fs_inode_by_name(struct inode * dir, struct dentry *dentry);
extern struct inode *j4fs_alloc_inode(struct super_block *sb);
extern void j4fs_destroy_inode(struct inode *inode);
extern void j4fs_read_inode (struct inode * inode);
extern int  j4fs_init(void);
extern void print_j4fs_inode(struct j4fs_inode *raw_inode);
extern int j4fs_get_block(struct inode *inode, sector_t iblock, struct buffer_head *bh_result,  int create);
extern int j4fs_get_blocks(struct inode *inode, sector_t iblock, unsigned long maxblocks, struct buffer_head *bh_result,  int create);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
extern int j4fs_permission(struct inode *inode, int mask);
#else
extern int j4fs_permission(struct inode *inode, int mask, struct nameidata *nd);
#endif

extern int j4fs_readpage_unlock(struct file *f, struct page *page);
extern int j4fs_readpage_nolock(struct file *f, struct page *page);
extern int j4fs_file_write(struct file *f, const char *buf, size_t n,loff_t *pos);
extern int j4fs_hold_space(int size);

extern void msleep(unsigned int msecs);

extern const struct inode_operations j4fs_file_inode_operations;
extern const struct file_operations j4fs_file_operations;
extern const struct inode_operations j4fs_dir_inode_operations;
extern const struct file_operations j4fs_dir_operations;
extern const struct address_space_operations j4fs_aops;
extern const struct super_operations j4fs_sops;

static inline struct j4fs_inode_info *J4FS_I(struct inode *inode)
{
	return container_of(inode, struct j4fs_inode_info, vfs_inode);
}
#else
extern int j4fs_test(void);
extern int j4fs_open(void);
#endif


extern int fsd_open(j4fs_ctrl *);
extern int fsd_close(j4fs_ctrl *);
extern int fsd_read(j4fs_ctrl *);
extern int fsd_write(j4fs_ctrl *);
extern int fsd_delete(j4fs_ctrl *);
extern int fsd_special(j4fs_ctrl *);
extern int fsd_print_meta_data(void);
extern int fsd_read_ro_header(void);
extern int fsd_mark_invalid(void);
extern int fsd_reclaim(void);
extern int fsd_panic(void);
extern int is_invalid_j4fs_rw_start(void);
#ifdef J4FS_TRANSACTION_LOGGING
extern int fsd_initialize_transaction(void);
#endif

extern int FlashDevRead(j4fs_device_info *dev_ptr, DWORD offset, DWORD length, BYTE *buffer);
extern int FlashDevWrite(j4fs_device_info *dev_ptr, DWORD offset, DWORD length, BYTE *buffer);
extern int FlashDevErase(j4fs_device_info *dev_ptr);
extern int FlashDevMount(void);
extern int FlashDevUnmount(void);
extern int FlashDevSpecial(j4fs_device_info *dev_ptr, DWORD scmd);

#ifdef __KERNEL__
#define TOUT(p) printk p
#else
#define TOUT(p) printf p
#endif

