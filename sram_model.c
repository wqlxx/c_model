#include "sram_model.h"
#include "drv_tbl_reg.h"

uint8* sram_model_reg_wbit[MAX_LOCAL_CHIP_NUM][MAX_REG_NUM];
uint8* sram_model_tbl_wbit[MAX_LOCAL_CHIP_NUM][MAX_TBL_NUM];
bool sram_model_initialized_flag[MAX_LOCAL_CHIP_NUM];

/*record each reg and tbl's software address*/
cmodel_reg_info_t cmodel_regs_info[MAX_LOCAL_CHIP_NUM][MAX_REG_NUM];
cmodel_tbl_info_t cmodel_tbls_info[MAX_LOCAL_CHIP_NUM][MAX_TBL_NUM];

int32 sram_humber_model(uint8 chip_id)
{
  int32 ret = DRV_E_NONE;
	uint32 tbl_id = 0;
	uint32 reg_id = 0;
	uint32 idx;
	
	/*the following is some share mem table, so only malloc one time*/
	uint32 share_tbl_num;
	bool nhp_malloc_flg = FALSE;
	bool acl_share_mem_flag = FALSE, qos_share_mem_flag = FALSE;


	uint32 nexthop_dyn_tbl_id[] = {DS_NEXTHOP, DS_NEXTHOP8W};
	uint32 acl_mac_v4_mpls_tbl_id[] = {DS_MAC_ACL, DS_IPV4_ACL, DS_MPLS_ACM};
	uint32 qos_mac_v4_mpls_tbl_id[] = {DS_IPV4_QOS, DS_MPLS_QOS};

	/*c_model only support up to 2 same chip*/
	SRAM_CHIP_ID_VALID_CHECK(chip_id);
	
	/*check whether the chip's sram has been malloc or not*/
	if(sram_model_initialized_flag[chip_id])
	{
		return DRV_E_NONE;
	}

	/*malloc each register memory*/
	for(reg_id = 0; req_id < MAX_REG_NUM; reg_id++)
	{
		uint32 reg_mem_size;
		
		/*malloc the reg's model memory according to the registed infomation*/
		reg_mem_size = drv_regs_list[reg_id].entry_size * drv_regs_list[reg_id].max_index_num;
		
		cmodel_regs_info[chip_id][reg_id].sw_data_base = (uint32)malloc(reg_mem_size);
		if(!cmodel_regs_info[chip_id][reg_id].sw_data_base)
		{
			return DRV_E_NO_MEMORY;
		}
		kal_memset((uint32 *)cmodel_reg_info[chip_id][reg_id].sw_data_base, 0, reg_mem_size)
	}

	/*malloc each tabel memory*/
	/*do not include dynicTbl/TcamKeyTabel/HashKeyTbl etc memory*/
	for(tbl_id = 0; tbl_id < MAX_TBL_NUM; tbl_id++)
	{
		uint32 tbl_mem_size;
		uint32* tbl_arry_ptr = NULL;
	
		/*check whether the table is dynic/TcamKey/HashKey ect or not */
		/*tcamkey wiil malloc in tcam model*/
		/*dymic table and hashkey allocation happens after driver init, and use cli cmd method*/
	
		/*those no-allocated tables not malloc*/
		if(!drv_tbls_list[tbl_id].max_index_num)
		{
			continue;
		}

		/*tcam each key not alloc mem in here*/
		if(SRAM_MODEL_IS_TCAM_KEY(tbl_id))
		{
			continue;
		}
		
		/*share tables had been malloc one tiem*/
		/*for those share tables, need to do specail resolve*/
		if(SRAM_MODEL_IS_NEXTHOP_SHARE_TBL(tbl_id))
		{
			if(nhp_malloc_flg == FALSE)
			{
				share_tbl_num = sizeof(nexthop_dyn_tbl_id)/sizeof(uint32);
				nhp_malloc_flg = TRUE;	
				tbl_arry_ptr = nexthop_dyn_tbl_id;
			}eles{
				continue;
			}
		}else if(SRAM_MODEL_IS_ACL_SHARE_TBL(tbl_id)){
			if(acl_share_mem_flag == FALSE)
			{
				share_tbl_num = sizeof(acl_mac_v4_mpls_tbl_id)/sizeof(uint32);
				acl_share_mem_flag = TRUE;
				tbl_arry_tbl = acl_mac_v4_mpls_tbl_id;
			}else{
				continue;
			}
		}else if(SRAM_MODEL_IS_QOS_SHARE_TBL(tbl_id)){
			if(qos_share_mem_flag == FALSE)
			{
				share_tbl_num = sizeof(qos_mac_v4_mpls_tbl_id)/sizeof(uint32);
				qos_share_mem_flag = TRUE;
				tbl_arry_tbl = qos_mac_v4_mpls_tbl_id;
			}else{
				continue;
			}
		}else{
			share_tbl_num = 0;
		}

		tbl_mem_size = drv_tbls_list[tbl_id].max_index_num * drv_tbls_list[tbl_id].entry_size;

		cmodel_tbls_info[chip_id][tbl_id].sw_data_base = (uint32)malloc(tbl_mem_size);
		if(!cmodel_tbls_info[chip_id][tbl_id].sw_data_base)
		{
			return DRV_E_NO_MEMORY;
		}
		
		if(SRAM_MODEL_IS_HASH_KEY_SHARE_TBL(tbl_id))/*hash tabel init 1*?
		{
			kal_memset((uint32*)cmodel_tbls_info[chip_id][tbl_id].sw_data_base, 1, tbl_mem_size);
		}else{
			kal_memset((uint32*)cmodel_tbls_info[chip_id][tbl_id].sw_data_base, 0, tbl_mem_size);
		}

		sram_model_tbl_wbit[chip_id][tbl_id] = malloc(drv_tbls_list[tbl_id].max_index_num);
		if(!sram_model_tbl_wbit[chip_id][tbl_id])
		{
			return DRV_E_NO_MEMORY;
		}
		kal_memset(sram_model_tbl_wbit[chip_id][tbl_id], 0, drv_tbls_list[tbl_id].max_index_num);
	
		/*record base of those tables which is share with the table*/
		if(share_tbl_num != 0)
		{
			for(idx = 0; idx < share_tbl_num; idx++)
			{
				uint32 tbl_id_temp = tbl_arry_ptr[idx];
				if(tbl_id_temp != tbl_id)
				{
					cmodel_tbls_info[chip_id][tbl_id_temp].sw_data_base = cmodel_tbls_info[chip_id][tbl_id].sw_data_base;
					sram_model_tbl_wbit[chip_id][tbl_id_temp] = sram_model_tbl_wbit[chip_id][tbl_id];
				}
			}
		}
	}
	sram_model_initialized_flag[chip_id] = TRUE;
	return ret;
}


int32 sram_model_initialize(uint8 chip_id)
{
	int32 ret = DRV_E_NONE;
	ret = sram_humber_model_initilialize(chip_id);	
	return ret;
}

int32 sram_model_release(uint8 chip_id)
{
	uint32 tbl_id = 0;
	uint32 reg_id = 0;
	uint32* sw_mem_ptr = NULL;
		
	/*cmodel only support up to 2 same chip*/
	SRAM_CHIP_ID_VALID_CHECK(chip_id);
	
	/*release each reg's memory*/
	for(reg_id = 0; reg_id < MAX_REG_NUM; reg_id++)
	{	
		if(!cmodel_regs_info[chip_id][reg_id].sw_data_base)
		{
			sw_mem_ptr = (uint32*)cmodel_regs_info[chip_id][reg_id].sw_data_base;	
			free(sw_mem_ptr);
			cmodel_regs_info[chip_id][reg_id].sw_data_base = 0;
		}
	
		if(!sram_model_reg_wbit[chip_id][reg_id])
		{
			free(sram_model_reg_wbit[chip_id][reg_id]);
			sram_model_reg_wbit[chip_id][reg_id] = NULL;
		}
	}
		
	/*release each tbl's memory*/
	for(tbl_id = 0; tbl_id < MAX_TBL_NUM; tbl_id++)
	{
		if(!cmodel_tbl_info[chip_id][tbl_id].sw_data_base)
		{
			sw_mem_ptr = (uint32*)cmodel_tbls_info[chip_id][tbl_id].sw_data_base;	
			free(sw_mem_ptr);
			cmodel_tbls_info[chip_id][tbl_id].sw_data_base = 0;
		}
	
		if(!sram_modeltbl_wbit[chip_id][tbl_id])
		{
			free(sram_model_tbl_wbit[chip_id][tbl_id]);
			sram_model_tbl_wbit[chip_id][tbl_id] = NULL;
		}
	}									
		
	return DRV_E_NONE;
}


int32 sram_model_read(uint8 chip_id,
			uint32 sw_model_addr,
			uint32 *data,
			int32 len);
{
	/*cmodel only support up to 2 same chip*/
	SRAM_CHIP_ID_VALID_CHECK(chip_id);

	/*may be add code to check whether the adderss is valid or not*/
	SRAM_MODEL_PTR_VALID_CHECK(data);
	kal_memcpy(data, (uint32*)sw_model_addr, len);
		
	reutrn DRV_E_NONE;
}


int32 sram_model_write(uint8 chip_id,
			uint32 sw_model_addr,
			uint32 *data,
			int32 len);	
{
	/*cmodel only support up to 2 same chip*/
	SRAM_CHIP_ID_VALID_CHECK(chip_id);

	/*may be add code to check whether the adderss is valid or not*/
	SRAM_MODEL_PTR_VALID_CHECK(data);
	kal_memcpy(uint32*)sw_model_addr, data, len);
		
	reutrn DRV_E_NONE;
}

int32 sram_model_reset(uint8 chip_id)
{
	uint32 tbl_id = 0;
	uint32 reg_id = 0;
	
	for(reg_id = 0; reg_id < MAX_REG_NUM; reg_id++)
	{
		uitn32 reg_mem_size = drv_regs_list[reg_id].entry_size * drv_regs_list[reg_id].max_index_num;
		
		if(cmodel_regs_info[chip_id][reg_id].sw_data_base)
		{
			kal_memset((uint32*)cmodel_regs_info[chip_id][reg_id].sw_data_base, 0, reg_mem_size);
		}

		if(cmodel_reg_wbit[chip_id][reg_id])
		{
			kal_memset(sram_cmodel_regs_info[chip_id][reg_id], 0, drv_regs_list[reg_id].max_index_num);
		}		
	}

	for(tbl_id = 0; tbl_id < MAX_TBL_NUM; tbl_id++)
	{
		uitn32 tbl_mem_size = drv_tbls_list[tbl_id].entry_size * drv_tbls_list[tbl_id].max_index_num;
		
		if(cmodel_tbls_info[chip_id][tbl_id].sw_data_base)
		{
			kal_memset((uint32*)cmodel_tbls_info[chip_id][tbl_id].sw_data_base, 0, tbl_mem_size);
		}

		if(cmodel_tbl_wbit[chip_id][tbl_id])
		{
			kal_memset(sram_cmodel_tbls_info[chip_id][tbl_id], 0, drv_tbls_list[tbl_id].max_index_num);
		}		
	}
	return DRV_E_NONE;
}
