int32 tcam_model_init(uint8 chip_id)
{
  TCAM_CHIP_ID_VALID_CHECK(chip_id);
	max_int_tcam_data_base = INT_TCAM_DATA_ASIC_BASE
				+ (INTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTS);
	max_int_tcam_mask_base = INT_TCAM_DATA_ASIC_BASE
				+ (INTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTS);
	max_ext_tcam_data_base = EXT_TCAM_DATA_ASIC_BASE
				+ (EXTERNAL_MAX_TCAM_ENTRY_NUM * EACH_TCAM_ENTRY_SW_SIM_BYTS);
	max_ext_tcam_data_base = EXT_TCAM_DATA_ASIC_BASE
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
