int32 tcam_model_init(uint8 chip_id)
{
	TCAM_CHIP_ID_VALID_CHECK(chip_id);
	max_int_tcam_data_base = INT_TCAM_DATA_ASIC_BASE
				+ (INTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTS);
	max_int_tcam_mask_base = INT_TCAM_MASK_ASIC_BASE
				+ (INTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTS);
	max_ext_tcam_data_base = EXT_TCAM_DATA_ASIC_BASE
				+ (EXTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTS);
	max_ext_tcam_mask_base = EXT_TCAM_MASK_ASIC_BASE
				+ (EXTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTS);
	
	if(tcam_model_inited[chip_id])
	{
		return DRV_E_NONE;
	}

	int_tcam_data_base[chip_id] = malloc(INTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTS);
	int_tcam_mask_base[chip_id] = malloc(INTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTS);
	int_tcam_wbit[chip_id] = malloc(INTERNAL_MAX_TCAM_ENTRY_NUM / 8);

	ext_tcam_data_base[chip_id] = malloc(EXTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTS);
	ext_tcam_mask_base[chip_id] = malloc(EXTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTS);
	ext_tcam_wbit[chip_id] = malloc(EXTERNAL_MAX_TCAM_ENTRY_NUM / 8);

	if(!int_tcam_data_base[chip_id] || !int_tcam_mask_base[chip_id]
	|| !int_tcam_wbit[chip_id] || !ext_tcam_data_base[chip_id]
	|| !ext_tcam_mask_base[chip_id] || !ext_tcam_wbit[chip_id])
	{
		exit(1);
	}

	memset(int_tcam_data_base[chip_id], 0, INTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTS);
	memset(int_tcam_mask_base[chip_id], 0, INTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTS);
	memset(int_tcam_wbit[chip_id], 0, INTERNAL_MAX_TCAM_ENTRY_NUM / 8);
	memset(ext_tcam_data_base[chip_id], 0, EXTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTS);
	memset(ext_tcam_mask_base[chip_id], 0, EXTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTS);
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
			if(!IS_BIT_SET(*(model_wbit_base + (offset/EACH_TCAM_ENTRY_SW_SIM_BYTS + i)/32),
						 (offset/EACH_TCAM_ENTRY_SW_SIM_BYTS + i)%32))
			{
				SET_BIT(*(model_wbit_base + (offset/EACH_TCAM_ENTRY_SW_SIM_BYTS + i)/32),
						 (offset/EACH_TCAM_ENTRY_SW_SIM_BYTS + i)%32);
			}
		}
	}else{
			if(!IS_BIT_SET(*(model_wbit_base + (offset/EACH_TCAM_ENTRY_SW_SIM_BYTS)/32),
						 (offset/EACH_TCAM_ENTRY_SW_SIM_BYTS)%32))
			{
				SET_BIT(*(model_wbit_base + (offset/EACH_TCAM_ENTRY_SW_SIM_BYTS)/32),
						 (offset/EACH_TCAM_ENTRY_SW_SIM_BYTS)%32);
			}
	}
	return DRV_E_NONE;		
}
