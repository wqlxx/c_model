#include "drv_io.h"

extern int32
drv_tbl_ioctl(uint8 chip_id, int32 index, uint32 cmd, void* val)
{
  int32 action;
	tbl_entry_t entry;
	tbl_id_t tbl_id;
	uint16 field_id;
	uint32 data_entry[MAX_ENTRY_WORD] = {0}, mask_entry[MAX_ENTRY_WORD] = {0};
	tables_t* tbl_ptr = NULL;

	DRV_CHIP_ID_VALID_CHECK(chip_id);
	DRV_PTR_VALID_CHECK(val);

	action = DRV_IOC_OP(cmd);
	tbl_id = DRV_IOC_MEMID(cmd);
	field_id = DRV_IOC_FIELDID(cmd);

	switch(tbl_id)
	{
		case DS_POLICER:
		case DS_FORWARDING_STATS:
			if(drv_io_api[chip_id].drv_indirect_sram_tbl_ioctl)
			{
				DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_indirect_sram_tbl_ioctl(chip_id, index, cmd, val));
				return DRV_E_NONE;
			}
			break;
		default:
 			break;
		}

		kal_memset(&entry, 0, sizeof(entry));
		entry.data_entry = data_entry;
		entry.mask_entry = mask_entry;
		
		DRV_TBL_ID_VALID_CHECK(tbl_id);
		
		tbl_ptr = DRV_TBL_GET_INPORT(tbl_id);
		
		//operating all the entry
		if(DRV_ENTRY_FLAG == field_id)
		{
			switch(action)
			{
				case DRV_IOC_WRTIE:
					if(INVALID_MASK_OFFSET == tbl_ptr->hw_mask_base)
					{
						DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_tbl_ds_to_entry(tbl_id, val, data_entry));
						if( drv_io_api[chip_id].drv_sram_tbl_write)
						{
							DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_tbl_write(chip_id, tbl_id, index, data_entry));
						}
					}else{
						DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_tbl_ds_to_entry(tbl_id, val, (tbl_entry_t*)&entry));
						if( drv_io_api[chip_id].drv_sram_tbl_write)
						{
							DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_tbl_write(chip_id, tbl_id, index, &entry));
						}
					}
					break;
				case DRV_IOC_READ:
					if(INVALID_MASK_OFFSET == tbl_ptr->hw_mask_base)
					{
				
						if( drv_io_api[chip_id].drv_sram_tbl_read)
						{
							DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_tbl_read(chip_id, tbl_id, index, (uint32*)data_entry));
						}
						DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_tbl_entry_to_ds(tbl_id, (uint32*)data_entry, val));
					}else{
						if( drv_io_api[chip_id].drv_sram_tbl_read)
						{
							DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_tbl_read(chip_id, tbl_id, index, &entry));
						}
						DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_tbl_ds_to_entry(tbl_id, (tbl_entry_t*)&entry, val));
					}
					break;	
				default:
					break;
			}
		}
		else
		{
			switch(action)
			{
				case DRV_IOC_WRTIE:
					if(INVALID_MASK_OFFSET == tbl_ptr->hw_mask_base)
					{
						if( drv_io_api[chip_id].drv_sram_tbl_read)
						{
							DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_tbl_read(chip_id, tbl_id, index, data_entry));
						}
						DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_tbl_field_set(tbl_id, field_id, data_entry, *(uint32*)val));
						if( drv_io_api[chip_id].drv_sram_tbl_write)
						{
							DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_tbl_write(chip_id, tbl_id, index, data_entry));
						}
					}else{
						if( drv_io_api[chip_id].drv_sram_tbl_read)
						{
							DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_tbl_read(chip_id, tbl_id, index, &entry));
						}
						DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_tbl_field_set(tbl_id, field_id, entry.data_entry, *(uint32*)val));
						if( drv_io_api[chip_id].drv_sram_tbl_write)
						{
							DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_tbl_write(chip_id, tbl_id, index, &entry));
						}
					}
					break;
				case DRV_IOC_READ:
					if(INVALID_MASK_OFFSET == tbl_ptr->hw_mask_base)
					{
				
						if( drv_io_api[chip_id].drv_sram_tbl_read)
						{
							DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_tbl_read(chip_id, tbl_id, index, data_entry));
						}
						DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_tbl_field_get(tbl_id, field_id, data_entry, (uint32*)val));
					}else{
						if( drv_io_api[chip_id].drv_sram_tbl_read)
						{
							DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_tbl_read(chip_id, tbl_id, index, &entry));
						}
						DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_tbl_field_get(tbl_id, field_id, entry.data_entry, (uint32*)val));
					}
					break;	
				default:
					break;
			}

		}						
	return DRV_E_NONE;																	
}

extern int32
drv_reg_ioctl(uint8 chip_id, int32 index, uint32 cmd, void* val);
{
	int32 action;
	tbl_id_t tbl_id;
	uint16 field_id;
	uint32 data_entry[MAX_ENTRY_WORD] = {0};
	int32 size;
	reg_id_t reg_id;

	DRV_CHIP_ID_VALID_CHECK(chip_id);
	DRV_PTR_VALID_CHECK(val);

	action = DRV_IOC_OP(cmd);
	tbl_id = DRV_IOC_MEMID(cmd);
	field_id = DRV_IOC_FIELDID(cmd);

	DRV_REG_ID_VALID_CHECK(reg_id);
	size = DRV_REG_ENTRY_SIZE(reg_id);
	
	kal_memset((uint8*)entry, 0, sizeof(entry));

	if(DRV_ENTRY_FLAG ==field_id)
	{
		regsiters_t *reg_ptr = NULL;
		reg_ptr = DRV_REG_GET_INFOPTR(reg_id);	
		
		switch(action)
		{
			case DRV_IOC_WRITE:	
				DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_reg_ds_to_entry(reg_id, val, entry));
				if(drv_regs_list[reg_id].hw_data_base >= TCAM_EXT_REG_RAM_OFFSET)
				{
					if(drv_io_api[chip_id].drv_ext_tcam_reg_write)
					{
						DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_ext_tcam_reg_write(chip_id, reg_id, index, val));
						return DRV_E_NONE;
					}
				}
				if(drv_io_api[chip_id].drv_sram_reg_write)
				{
					DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_reg_write(chip_id, reg_id, index, entry));
				}
				break;
		
			case DRV_IOC_READ:
				if(drv_regs_list[reg_id].hw_data_base >= TCAM_EXT_REG_RAM_OFFSET)
				{
					if(drv_io_api[chip_id].drv_ext_tcam_reg_read)
					{
						DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_ext_tcam_reg_read(chip_id, reg_id, index, entry));
						return DRV_E_NONE;
					}	
	
				}
			
				if(drv_io_api[chip_id].drv_sram_reg_read)
				{
					DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_reg_read(chip_id, reg_id, index, entry));
				}
				DRV_IF_ERROR_RETURN(drv_io_api[chip_id].drv_sram_reg_entry_to_ds(reg_id, entry, val));
				break;
				
}


extern int32
drv_tcam_tbl_remove(uint8 chip_id, int32 tbl_id, uint32 index);

extern int32
drv_hash_key_ioctl(uint8 chip_id, uint32 tbl_id, uint32 tbl_idx, uint32 *key, hash_op_type_e operation_type);

extern int32
drv_hash_key_lkup_index(uint8 chip_id, cpu_reg_hash_key_type_e cpu_hashkey_type, uint32* key, hash_ds_ctl_cpu_key_status_t * hash_cpu_stats);
