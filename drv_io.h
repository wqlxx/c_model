#ifndef _DRV_IO_H_
#define _DRV_IO_H_ 1

#include "drv_enum.h"
#include "drv_common.h"
#include "drv_humber.h"

#define MAX_LOCAL_CHIP_NUM 2
#define  MAX_LOCAL_PORT_NUM 256

#define MAX_ENTRY_WORD 64
#define MAX_ENTRY_BYTE 128

#define CONDITIONAL_BREAK(exp) if((exp)) break
#define CONDITIONAL_CONTINUE(exp) if((exp)) continue

#define DRV_IOC_DIR_BITS 2
#define DRV_IOC_MEM_BITS 1
#define DRV_IOC_MEMID_BITS 13
#define DRV_IOC_FIELDID_BITS 16
#define DRV_ENTRY_FLAGS  0x1fff

#define DRV_HASH_INVALID_INDEX 0x1FFFF
#define DRV_IOC_DIR_MASK	((1 << DRV_IOC_DIR_BITS)-1)
#define DRV_IOC_MEM_MASK	((1 << DRV_IOC_MEM_BITS)-1)
#define DRV_IOC_MEMID_MASK	((1 << DRV_IOC_MEMID_BITS)-1)
#define DRV_IOC_FIELDID_MASK	((1 << DRV_IOC_FIELDID_BITS)-1)
#define DRV_IOC_FIELDID_SHIFT	0
#define DRV_IOC_MEMID_SHIFT	((DRV_IOC_FIELDID_SHIFT + DRV_IOC_FIELDID_BITS)
#define DRV_IOC_MEM_SHIFT	((DRV_IOC_MEMID_SHIFT + DRV_IOC_MEMID_BITS)
#define DRV_IOC_DIR_SHIFT	((DRV_IOC_MEM_SHIFT + DRV_IOC_MEM_BITS)
#define DRV_IOC_OP(cmd)		(((cmd) >> DRV_IOC_DIR_SHIFT)&DRV_IOC_DIR_MASK)
#define DRV_IOC_MEMID(cmd)	(((cmd) >> DRV_IOC_MEMID_SHIFT)&DRV_IOC_MEMID_MASK)
#define DRV_IOC_FIELDID(cmd)	(((cmd) >> DRV_IOC_FIELDID_SHIFT)&DRV_IOC_FIELDID_MASK)

#define CHIP_IS_EXTERNAL_CHIP(chip_id) ((chip_id) == current_external_chip_id;//0xff

#define DRV_IOC_READ 1U
#define DRV_IOC_WRITE 2U

#define DRV_IOC(dir, mem, memid, fieldid)\
	(((dir)<<DRV_IOC_DIR_SHIFT) | \
	((mem)<<DRV_IOC_MEM_SHIFT) |\
	((memid)<<DRV_IOC_MEMID_SHIFT) |\
	((fields)<<DRV_IOC_FIELDS_SHIFT))

typedef int32 mutex_t;

//tcam data mask storage structure
struct tbl_entry_s
{
	uint32* data_entry;
	uitn32* mask_entry;
}
typedef struct tbl_entry_s tbl_entry_t;

#define DRV_IF_ERR_RETURN(op) \
	{			\
		int rv;		\
		if( (rv = (op)) < 0)	\
		{			\
			return(rv);	\
		}\
	}

#define DRV_PTR_VALID_CHECK(ptr)	\
	if(NULL == (ptr))		\
	{				\
		return DRV_E_INVALID_PTR;\
	}

#define DRV_IOR(mem, memid, fieldid) \
	DRV_IOC(DRV_IOC_READ, (mem), (memid), (fieldid))
#define DRV_IOR(mem, memid, fieldid) \
	DRV_IOC(DRV_IOC_WRITE, (mem), (memid), (fieldid))

struct hash_ds_ctl_cpu_key_status_s
{
	uint32 cpu_lu_index	:17;
	uint32 rsv_0 		:14;
	uint32 cpu_key_hit	:1;
};
typedef struct hash_ds_ctl_cpu_key_status_s hash_ds_ctl_cpu_key_status_t;
enum cpu_reg_hash_key_type
{
	CPU_HASH_KEY_TYPE_MAC_DA = 0,
	CPU_HASH_KEY_TYPE_MAC_SA = 1,
	CPU_HASH_KEY_TYPE_IPV4_DA = 2,
	CPU_HASH_KEY_TYPE_IPV4_SA = 3,
	CPU_HASH_KEY_TYPE_IPV6_DA = 4,
	CPU_HASH_KEY_TYPE_IPV6_SA = 5,
	CPU_HASH_KEY_TYPE_RESERVED0 = 6,
	CPU_HASH_KEY_TYPE_RESERVED1 = 7
};
typedef enum cpu_req_hash_key_type cpu_req_hash_key_type_e;

enum hash_op_type
{
	HASH_OP_TP_ADD_ENTRY = 0,
	HASH_OP_TP_DEL_ENTRY_BY_KEY = 1,
	HASH_OP_TP_DEL_ENTRY_BY_INDEX = 2,
	HASH_OP_TP_MAX_VALUE = 3
};
typedef enum hash_op_type hash_op_type_e;



extern int32
drv_tbl_ioctl(uint8 chip_id, int32 index, uint32 cmd, void* val);

extern int32
drv_reg_ioctl(uint8 chip_id, int32 index, uint32 cmd, void* val);

extern int32
drv_tcam_tbl_remove(uint8 chip_id, int32 tbl_id, uint32 index);

extern int32
drv_hash_key_ioctl(uint8 chip_id, uint32 tbl_id, uint32 tbl_idx, uint32 *key, hash_op_type_e operation_type);

extern int32
drv_hash_key_lkup_index(uint8 chip_id, cpu_reg_hash_key_type_e cpu_hashkey_type, uint32* key, hash_ds_ctl_cpu_key_status_t * hash_cpu_stats):

#endif
