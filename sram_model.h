#ifndef _SRAM_MODEL_H_
#define _SRAM_MODEL_H_ 1

#include "drv_error.h"
#include "drv_enum.h"
#include "drv_humber.h"
#include "drv_common.h"
#include "kal.h"

#define SRAM_MODEL_PTR_VALID_CHECK(ptr)   \
  if( NULL == (PTR) )                 \
      	{
		return DRV_E_INVALID_PTR;	\
	}					

#define SRAM_MODEL_IS_TCAM_KEY(tbl_id)			\
		((DS_MAC_KEY == (tbl_id))		\
		||(DS_ACL_MAC_KEY == (tbl_id))		\
		||(DS_ACL_IPV4_KEY == (tbl_id))	\
		||(DS_ACL_IPV6_KEY == (tbl_id))	\
		||(DS_QOS_MAC_KEY == (tbl_id))		\
		||(DS_QOS_IPV4_KEY == (tbl_id))	\
		||(DS_QOS_IPV6_KEY == (tbl_id)))	

#define SRAM_MODEL_IS_ACL_SHARE_TBL(tbl_id)		\
	((DS_MAC_ACL== (tbl_id))			\
	||(DS_IPV4_ACL == (tbl_id))			\
	||(DS_MPLS_ACL == (tbl_id)))

#define SRAM_MODEL_IS_QOS_SHARE_TBL(tbl_id)		\
		((DS_IPV4_QOS == (tbl_id))		\
		||(DS_MPLS_QOS == (tbl_id))		
			
#define SRAM_CHIP_ID_VALID_CHECK(chip_id)	\
	if( (chip_id) >= (MAX_LOCAL_CHIP_NUM))	\
	{					\
		DRV_DBG_INFO("out of chip_id num");	\
		return ERV_E_INVALID_CHIP;		\
	}					


/*recode each register sw base address in cmodel structure*/
struct cmodel_reg_info_s
{
	uint32 sw_data_base;
};
typedef struct cmodel_reg_info_s cmodel_reg_info_t;


/*record each table sw base address in cmodel structure*/
/*when the table is tcam key, we can get the key  database and maskbase
 according to the whole tcam base and the key hw_offset, so to tcam key table
 we do need the sw_data_base*/
struct cmodel_tbl_info_s
{
	uint32 sw_data_base;
};
typedef struct cmodel_tbl_info_s cmodel_tbl_info_t;


extern int32 sram_model_initialize(uint8 chip_id);
extren int32 sram_model_release(uint8 chip_id);

extern int32 sram_model_read(uint8 chip_id,
				uint32 sw_model_addr,
				uint32 *data,
				int32 len);

extern int32 sram_model_write(uint8 chip_id,
				uint32 sw_model_addr,
				uint32 *data,
				int32 len);

extern	int32 sram_model_reset(uint8 chip_id);

#define MAX_REG_NUM 2541
#define MAX_TBL_NUM 294

extern cmodel_reg_info_t cmodel_regs_info[MAX_LOCAL_CHIP_NUM][MAX_REG_NUM];
extern cmodel_tbl_info_t cmodel_tbls_info[MAC_LOCAL_CHIP_NUM][MAX_REG_NUM];

struct fields_s
{
	uint32 len;
	uint8 word_offset;
	uint8 bit_offset;
	char* field_nam;
};
typedef struct fields_s fields_t;

/*table data structure*/
struct tables_s
{
	uint32 hw_mask_base;
	uint32 hw_data_base;
	uint32 max_index_num;
	
	uint8 key_size;
	uint8 entry_size;
	uint8 num_fields;
	fields_t *ptr_fields;
};
typedef struct tables_s tables_t;

struct registers_s
{
	uint32 hw_data_base;
	uint32 max_index_num;
	uint16 entry_size;
	uint8 num_fields;
	fields_t *ptr_fields;
};
typedef struct registers_s registers_t; 

registers_t drv_regs_list[MAX_REG_NUM];
tables_t drv_tbls_list[MAX_TBL_NUM];

#endif				
