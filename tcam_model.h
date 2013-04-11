#ifndef _TCAM_MODEL_H_
#define _TCAM_MODEL_H_ 1

#include "drv_cfg.h"
#include "drv_hunber.h"
#include "drv_tbl_reg.h"

#define EXTERNAL_MAX_TCAM_ENTRY_NUM (1024 * 256)
#define INTERNAL_MAX_TCAM_ENTRY_NUM (1024 * 16)

#define INT_TCAM_DATA_ASIC_BASE 0x11000000
#define INT_TCAM_MASK_ASIC_BASE 0x12000000

#define EXT_TCAM_DATA_ASIC_BASE 0x13000000
#define EXT_TCAM_MASK_ASIC_BASE 0x14000000

/*In software, use 128 bits(16 bytes) to simulate the tcam 80 bits entry*/
#define EACH_TCAM_ENTRY_SW_SIM_BYTES 16

#define TCAM_MODEL_PTR_VALID_CHECK(ptr)\
  if( NULL == (ptr))	\
	{			\
		return DRV_E_INVALID_PTR;\
	}

/*chip id check macro define*/
#define TCAM_CHIP_ID_VALID_CHECK(chip_id)\
	if( (chip_id) >= (MAX_LOCAL_CHIP_NUM))\
	{					\
		return DRV_E_INVALID_CHIP;	\
	}

typedef enum tcam_data_mask_e
{
	TCAM_DATA,
	TCAM_MASK
}tcam_data_mask_t;

/*the following give a min abstract tcam api*/
extern int32 tcam_model_init(uin8 chip_id);

extern int32 tcam_model_release(uin8 chip_id);

extern int32 tcam_model_write(uin8 chip_id, uint32 sw_model_addr, uint32 *data, int32 len);

extern int32 tcam_model_read(uin8 chip_id, uint32 sw_model_addr, uint32 *data, int32 len);

extern int32 tcam_model_lookup(uin8 chip_id, uint32 tbl_id, uint32 *data, int32 idx);

extern int32 tcam_model_dump();

extern int32 tcam_model_remove(uin8 chip_id, uint32 sw_model_addr);

extern uint32* int_tcam_data_base[MAX_LOCAL_CHIP_NUM];
extern uint32* int_tcam_mask_base[MAX_LOCAL_CHIP_NUM];
extern uint32* int_tcam_wbit[MAX_LOCAL_CHIP_NUM];

extern uint32* ext_tcam_data_base[MAX_LOCAL_CHIP_NUM];
extern uint32* ext_tcam_mask_base[MAX_LOCAL_CHIP_NUM];
extern uint32* ext_tcam_wbit[MAX_LOCAL_CHIP_NUM];

#endif
