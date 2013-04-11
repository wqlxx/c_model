#include "tcam_model.h"

static bool tcam_model_inited[MAX_LOCAL_CHIP_NUM] = {FALSE};
uint32* int_tcam_data_base[chip_id] = {0};
uint32* int_tcam_mask_base[chip_id] = {0};
uint32* int_tcam_wbit[chip_id] = {0};

uint32* ext_tcam_data_base[chip_id] = {0};
uint32* ext_tcam_mask_base[chip_id] = {0};
uint32* ext_tcam_wbit[chip_id] = {0};

uint32 max_int_tcam_data_base = 0;
uint32 max_int_tcam_mask_base = 0;
uint32 max_ext_tcam_data_base = 0;
uint32 max_ext_tcam_mask_base = 0;

static bool tcam_model_addr_check(uint8 chip_id, uint32 model_addr, bool* in_internalm bool* is_data_addr)
{
	uint32 max_model_dataaddr, max_model_maskaddr;
	
	TCAM_CHIP_ID_VALID_CHECK(chip_id);
	TCAM_MODEL_PTR_VALID_CHECK(data);
	TCAM_MODEL_PTR_VALID_CHECK(idx);

	max_model_dataaddr = (uint32)int_tcam_data_base[chip_id]
			+ (INTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTES);
	max_model_maskaddr = (uint32)int_tcam_mask_base[chip_id]
			+ (INTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTES);
	
	if((model_addr >= (uint32)int_tcam_data_base[chip_id])
		&& (model_addr < max_model_dataaddr))
	{
		*in_internal = TRUE;
		*is_data_addr = TRUE;
		return TRUE;
	}

	if((model_addr >= (uint32)int_tcam_mask_base[chip_id])
		&& (model_addr < max_model_maskaddr))
	{
		*in_internal = TRUE;
		*is_data_addr = FALSE;
		return TRUE;
	}
	
	max_model_dataaddr = (uint32)ext_tcam_data_base[chip_id]
			+ (EXTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTES);
	max_model_maskaddr = (uint32)ext_tcam_mask_base[chip_id]
			+ (EXTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTES);
	
	if((model_addr >= (uint32)ext_tcam_data_base[chip_id])
		&& (model_addr < max_model_dataaddr))
	{
		*in_internal = FALSE;
		*is_data_addr = TRUE;
		return TRUE;
	}

	if((model_addr >= (uint32)ext_tcam_mask_base[chip_id])
		&& (model_addr < max_model_maskaddr))
	{
		*in_internal = FALSE;
		*is_data_addr = FALSE;
		return TRUE;
	}	
	
	return FALSE;
}

int32 tcam_model_init(uint8 chip_id)
{
	TCAM_CHIP_ID_VALID_CHECK(chip_id);
	max_int_tcam_data_base = INT_TCAM_DATA_ASIC_BASE
				+ (INTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTES);
	max_int_tcam_mask_base = INT_TCAM_MASK_ASIC_BASE
				+ (INTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTES);
	max_ext_tcam_data_base = EXT_TCAM_DATA_ASIC_BASE
				+ (EXTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTES);
	max_ext_tcam_mask_base = EXT_TCAM_MASK_ASIC_BASE
				+ (EXTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTES);
	
	if(tcam_model_inited[chip_id])
	{
		return DRV_E_NONE;
	}

	int_tcam_data_base[chip_id] = malloc(INTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTES);
	int_tcam_mask_base[chip_id] = malloc(INTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTES);
	int_tcam_wbit[chip_id] = malloc(INTERNAL_MAX_TCAM_ENTRY_NUM / 8);

	ext_tcam_data_base[chip_id] = malloc(EXTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTES);
	ext_tcam_mask_base[chip_id] = malloc(EXTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTES);
	ext_tcam_wbit[chip_id] = malloc(EXTERNAL_MAX_TCAM_ENTRY_NUM / 8);

	if(!int_tcam_data_base[chip_id] || !int_tcam_mask_base[chip_id]
	|| !int_tcam_wbit[chip_id] || !ext_tcam_data_base[chip_id]
	|| !ext_tcam_mask_base[chip_id] || !ext_tcam_wbit[chip_id])
	{
		exit(1);
	}

	memset(int_tcam_data_base[chip_id], 0, INTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTES);
	memset(int_tcam_mask_base[chip_id], 0, INTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTES);
	memset(int_tcam_wbit[chip_id], 0, INTERNAL_MAX_TCAM_ENTRY_NUM / 8);
	memset(ext_tcam_data_base[chip_id], 0, EXTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTSE);
	memset(ext_tcam_mask_base[chip_id], 0, EXTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTES);
	memset(ext_tcam_wbit[chip_id], 0, EXTERNAL_MAX_TCAM_ENTRY_NUM / 8);

	tcam_model_inited[chip_id] = TRUE;
	return DRV_E_NONE;
}
	
int32 tcam_model_release(uint8 chip_id)
{
	TCAM_CHIP_ID_VALID_CHECK(chip_id);

	free(int_tcam_data_base[chip_id]);
	int_tcam_data_base[chip_id] = NULL;
	
	free(int_tcam_mask_base[chip_id]);
	int_tcam_mask_base[chip_id] = NULL;

	free(int_tcam_wbit[chip_id]);
	int_tcam_wbit[chip_id] = NULL;

	free(ext_tcam_data_base[chip_id]);
	ext_tcam_data_base[chip_id] = NULL;

	free(ext_tcam_mask_base[chip_id]);
	ext_tcam_mask_base[chip_id] = NULL;

	free(ext_tcam_wbit[chip_id]);
	ext_tcam_wbit[chip_id] = NULL;

	tcam_model_inited[chip_id] = FALSE;

	return DRV_E_NONE;
}

int32 tcam_mode_write(uint8 chip_id, uint32 sw_model_addr, uint32 *data, int32 len)
{
	uint32 offset;
	uint32 model_data_base, model_mask_base;
	uint32 *model_wbit_base = NULL;
	bool in_internal = TRUE;
	bool is_data_addr = TRUE;
	uint32 i;
	
	/*check para*/
	TCAM_CHIP_ID_VALID_CHECK(chip_id);
	TCAM_MODEL_PTR_VALID_CHECK(data);

	if(!tcam_model_inited[chip_id];
	{
		return DRV_E_TCAM_RESET;
	}
	
	if(!tcam_model_addr_addr_check(chip_idm, sw_model_addr, &in_internalm, &id_data_addr))
	{
		return DRV_E_INVALID_ADDR;
	}

	if(in_internal)
	{
		model_data_base = (uint32)int_tcam_data_base[chip_id];
		model_mask_base = (uint32)int_tcam_mask_base[chip_id];
		model_wbit_base = int_tcam_wbit[chip_id];
	}else{
		model_data_base = (uint32)ext_tcam_data_base[chip_id];
		model_mask_base = (uint32)ext_tcam_mask_base[chip_id];
		model_wbit_base = ext_tcam_wbit[chip_id];
	}		
	
	memcpy((uint8*)sw_model_addr, data, len);

	if(is_data_addr)
	{
		offset = sw_model_addr - model_data_base;
	}else{
		offset = sw_model_addr - model_mask_base;
	}

	if(len >= EACH_TCAM_ENTRY_SW_SIM_BYTS)
	{
		/*setting Wbit(uint: 1 tcam entry), len(Bytes)*/
		for( i = 0; i < len/EACH_TCAM_ENTRY_SW_SIM_BYTS; i++)
		{
			/*model_wbit_base: each bit -- each 80 bits entry*/
			/*offset: address offset(bytes)*/
			/*offset/16: offset address space include the 80bit entry number (sw use 128 bit to asic 80 bits)*/
			if(!IS_BIT_SET(*(model_wbit_base + (offset/EACH_TCAM_ENTRY_SW_SIM_BYTES + i)/32),
						 (offset/EACH_TCAM_ENTRY_SW_SIM_BYTES + i)%32))
			{
				SET_BIT(*(model_wbit_base + (offset/EACH_TCAM_ENTRY_SW_SIM_BYTES + i)/32),
						 (offset/EACH_TCAM_ENTRY_SW_SIM_BYTES + i)%32);
			}
		}
	}else{
			if(!IS_BIT_SET(*(model_wbit_base + (offset/EACH_TCAM_ENTRY_SW_SIM_BYTES)/32),
						 (offset/EACH_TCAM_ENTRY_SW_SIM_BYTES)%32))
			{
				SET_BIT(*(model_wbit_base + (offset/EACH_TCAM_ENTRY_SW_SIM_BYTES)/32),
						 (offset/EACH_TCAM_ENTRY_SW_SIM_BYTES)%32);
			}
	}
	return DRV_E_NONE;		
}

int32 tcam_model_read(uint8 chip_id, uint32 sw_model_addr, uint32 *data, int32 len)
{
	bool in_internal = TRUE;
	bool is_data_addr = TRUE;
	
	/*check para*/
	TCAM_CHIP_ID_VALID_CHECK(chip_id);
	TCAM_MODEL_PTR_VALID_CHECK(data);

	/*before read tcam, check whether the tcam model is inited or not*/
	if(!tcam_model_inited[chip_id])
	{
		return DRV_E_TCAM_RESET;
	}

	if(!tcam_model_addr_check(chip_id, sw_model_addr, &in_internal, &is_data_addr))
	{
		return DRV_E_INVALID_ADDR;
	}

	memcpy(data, (uint8*)sw_model_addr, len);
	return DRV_E_NONE;
}

int32 tcam_model_remove(uint8 chip_id, uint32 sw_model_addr)
{
	bool in_internal, is_data_addr;
	uint32 model_data_base, model_mask_base, offset;
	uint32 *model_wbit_base = NULL;

	if(!tcam_model_addr_check(chip_id, sw_model_addr, &in_internal, &is_data_addr))
	{
		return DRV_E_INVALID_ADDR;
	}
	

	if(in_internal)
	{
		model_data_base = (uint32)int_tcam_data_base[chip_id];
		model_mask_base = (uint32)int_tcam_mask_base[chip_id];
		model_wbit_base = int_tcam_wbit[chip_id];
	}else{
		model_data_base = (uint32)ext_tcam_data_base[chip_id];
		model_mask_base = (uint32)ext_tcam_mask_base[chip_id];
		model_wbit_base = ext_tcam_wbit[chip_id];
	}
	
	if(is_data_addr)
	{
		offset = sw_model_addr - model_data_base;
	}else{
		offset = sw_model_addr - model_mask_base;
	}	

	if(!IS_BIT_SET(*(model_wbit_base + (offset/EACH_TCAM_ENTRY_SW_SIM_BYTES)/32),
				(offset/EACH_TCAM_ENTRY_SW_SIM_BYTES)%32))
	{
		CLEAR_BIT(*(model_wbit_base + (offset/EACH_TCAM_ENTRY_SW_SIM_BYTES)/32),
				(offset/EACH_TCAM_ENTRY_SW_SIM_BYTES)%32);
	}	

	return DRV_E_NONE;
}

int32 tcam_model_lookup(uin8 chip_id, uint32 tbl_id, uin32 *data, uin32 *idx)
{
	uint32 *sw_data_base = NULL;
	uint32 *sw_mask_base = NULL;
	uint32 *sw_vbit_base = NULL;	
	uint32 hw_data_base, hw_mask_base;
	uint32 *key_data_base = NULL, *key_mask_base = NULL;
	tables_t* tbl = NULL;
	uint32 i, j;
	bool in_internal = TRUE;
	
	/*check para*/
	TCAM_CHIP_ID_VALID_CHECK(chip_id);
	TCAM_MODEL_PTR_VALID_CHECK(data);
	TCAM_MODEL_PTR_VALID_CHECK(idx);
		
	if(!tcam_model_inited[chip_id])
	{
		return DRV_E_TCAM_RESET;
	}

	tbl = DRV_TBL_GET_INFOPTR(tbl_id);
	
	if(tbl->max_index_num == 0)
	{
		*idx = 0xffffffff;
		return DRV_E_NONE;
	}
	
	/*check whether the table is allocatebable into the internal tcam*/
	if(((tbl->hw_data_base >= INT_TCAM_DATA_ASIC_BASE) 
		&&(tbl->hw_data_base < max_int_tcam_data_base)))
	{
		in_internal = TRUE;
	}
	else if (((tbl->hw_data_base >= EXT_TCAM_DATA_ASIC_BASE)
		&&(tbl->hw_data_base < max_ext_tcam_data_base)))
	{
		in_internal = FALSE;
	}
	else
	{
		return DRV_E_INVALID_ADDR;
	}

	if(in_internal)
	{
		sw_data_base = int_tcam_data_base[chip_id];
		sw_mask_base = int_tcam_mask_base[chip_id];
		sw_vbit_base = int_tcam_wbit[chip_id];
		hw_data_base = INT_TCAM_DATA_ASIC_BASE;
		hw_mask_base = INT_TCAM_MASK_ASIC_BASE;
	}else{
		sw_data_base = ext_tcam_data_base[chip_id];
		sw_mask_base = ext_tcam_mask_base[chip_id];
		sw_vbit_base = ext_tcam_wbit[chip_id];
		hw_data_base = EXT_TCAM_DATA_ASIC_BASE;
		hw_mask_base = EXT_TCAM_MASK_ASIC_BASE;
	}

	key_data_base = sw_data_base + (tbl->hw_data_base - hw_data_base)/4;
	key_mask_base = sw_mask_base + (tbl->hw_mask_base - hw_mask_base)/4;

	for(i = 0; i < tbl->max_index_num; ++i)
	{	
		uint32 entry_num = i*(tbl->key_size/EACH_TCAM_ENTRY_SW_SIM_BYTES)
				+ (tbl->hw_data_base - hw_data_base)/EACH_TCAM_ENTRY_SW_SIM_BYTES;
		
	/*check the tcam entry's Wbit*/
	if(!(IS_BIT_SET(*(sw_vbit_base + entry_num/32), entry_num%32)))
	{
		continue;
	}

	/*compare key according to the word uint*/
	for(j = 0; j < tbl->key_size/4; ++j)
	{
		uint32 mask = *(key_mask_base + i * tbl->key_size/4 + j);
		uint32 data = *(key_data_base + i * tbl->key_size/4 + j);
		uint32 key = *(data + j);

		if((record & mask) != (key & mask))
		{
			break;
		}
		
		if(j == tbl->key_size/4 - 1)
		{
			*idx = (tbl->hw_data_base - hw_data_base)/EACH_TCAM_ENTRY_SW_SIM_BYTES	
				+ i*(tbl->key_size/EACH_TCAM_ENTRY_SW_SIM_BYTES);
			return DRV_E_NONE;
		}
	}
	*idx = 0xffffffff;
	return DRV_E_NOT_FOUND;
}


