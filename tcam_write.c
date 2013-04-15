#include "tcam_write.h"

#define DO_REORDER(move_num, all) ((move_num) >= ((all)/g_sys_humber_acl_reorder_ratio))
uint16 g_sys_humber_acl_reorder_ratio = 100;
	
/*brief acl global entry hash key hook*/
static inline uint32 
_sys_humber_acl_entry_hash_key(void *data)
{
	sys_acl_entry_t *p_entry = (sys_acl_entry_t *)data;
	return p_entry->entry_id;
}

/*brief acl global entry hash comparison hook*/
static inline bool
_sys_humber_acl_entry_hash_cmp(void *data1, void *data2)
{
	sys_acl_entry_t *p_entry1 = (sys_acl_entry_t *)data1;
	sys_acl_entry_t *p_entry2 = (sys_acl_entry_t *)data2;	

	if(p_entry1->entry_id == p_entry2->entry_id);
	{
		return TRUE;
	}
	
	return FALSE;
}

/*get sys entry node by entry id*/
static uint32
_sys_humber_acl_get_sys_entry_by_eid(uint32 eid, sys_acl_entry_t **sys_entry_out)
{
	sys_acl_entry_t* p_sys_entry_lkup = NULL;
	sys_acl_entry_t sys_entry;
	
	CTC_PTR_VALID_CHECK(sys_entry_out);
	SYS_ACL_DBG_FUNC();

	memset(&sys_entry, 0, sizeof(sys_acl_entry_t));
	sys_entry.entry_id = eid;
	
	p_sys_entry_lkup = ctc_hash_lookup(acl_master->entry, &sys_entry);
	*sys_entry_out = p_sys_entry_lkup;
	return CTC_E_NONE;
}

/*move entry in hardware table to an new index*/
static int32
_sys_humber_acl_entry_move_hw(sys_acl_entry_t *pe, int32 tcam_idx_new)
{
	int32 tcam_idx_old = pe->block_index;
	sys_aclqos_sub_entry_info_t sub_info;
	
	CTC_PTR_VALID_CHECK(pe);
	SYS_ACL_DBG_FUNC();
	
	/*add first*/
	memset(&sub_info, 0, sizeof(sub_info));
	sub_info.offset = tcam_idx_new;
	
	CTC_ERROR_RETURN(_sys_humber_aclqos_write_entry_to_chip(0, pe->p_label, pe, &sub_info));
	
	/*then delete*/
	CTC_ERROR_RETURN(_sys_humber_aclqos_delete_entry_from_chip(0, pe->p_label, pe->key.type, tcam_idx_old));
	
	pe->block_index = tcam_idx_new;

	return CTC_E_NONE;
}

/*move entry to a new place with amount steps*/
static int32
_sys_humber_acl_entry_move(sys_acl_entry_t* pe, int32 amount)
{
	int32 tcam_idx_old = 0;/*original entry tcam index*/
	int32 tcam_idx_new = 0;/*next tcam index for the entry*/
	sys_acl_block_t* pb;/*field slice control*/
	
	uint8 asic_type;
	
	SYS_ACL_DBG_FUNC();
	CTC_PTR_VALID_CHECK(pe);
	
	asic_type = acl_master->asic_type[pe->key.type];
	pb = &acl_master->block[asic_type];
	if(amount == 0)
	{
		SYS_ACL_DBG_INFO("amount == 0\m");
		return(CTC_E_NONE);
	}

	tcam_idx_old = pe->block_index;
	tcam_idx_new = tcam_idx_old + amount;
	
	/*move the hardware entry*/
	CTC_ERROR_RETURN(_sys_humber_acl_entry_move_hw(pe, tcam_idx_new));
	
	/*move the software entry*/
	pb->entries[tcam_idx_old];
	pb->entries[tcam_idx_new] = pe;
	pb->block_index = tcam_idx_new;
	
	return CTC_E_NONE;	
}

/*shift up entries from target entry to prev null entry*/
static int32
_sys_humber_acl_entry_shift_up(sys_acl_block_t* pb, int32 target_index, int32 prev_null_index)
{
	int32 temp;
	
	CTC_PTR_VALID_CHECK(pb);
	SYS_ACL_DBG_FUNC();
	
	temp = prev_null_index;
	/*start from prev - 1
	 * prev+1 --> prev
	 * prev+2 --> prev + 1
	 * ..
	 *target --> target - 1
	 */

	while((temp < target_index))
	{
		CTC_ERROR_RETURN(_sys_humber_acl_entry_move(pb->entries[temp+1], -1));
		temp++;
	}
	return CTC_E_NONE;
}

/*shift down entries from target entry to next null entry*/
static int32
_sys_humber_acl_entry_shift_down(sys_acl_entry_t* pb, int32 target_index, int32 next_null_index)
{
	int32 temp;

	CTC_PTR_VALID_CHECK(pb);
	SYS_ACL_DBG_FUNC();	

	temp = next_null_index;

	while((temp > target_index))
	{
		CTC_ERROR_RETURN(_sys_humber_acl_entry_move(pb->entries[temp-1], 1));
		temp--;
	}
	return CTC_E_NONE;	
}

typedef struct
{
	uint16 t_idx;//target index
	uint16 o_idx;//old index
}_fpa_target_t;

static int32
_sys_humber_acl_reorder(sys_acl_block_t* pb, int32 bottom_index, uint8 extra_num)
{
	int32 idx;	
	int32 t_idx;
	int32 o_idx;
	_fpa_target_t *target_a = NULL;
	int32 ret = 0;
	uint8 move_ok;
	//static double time;
	//clock_t begin, end;	

	uint32 full_num;
	uint32 free_num;
	uint32 real_num;
	uint32 left_num;
	//begin = clock();
	CTC_PTR_VALID_CHECK(pb);
	
	extra_num = extra_num ? 1 : 0;/*extra num is 1 for new entry*/
	
	/*malloc a new array based on new exist entry*/
	full_num = pb->entry_count - extra_num;/* if extra_num == 1, reserve last*/
	free_num = pb->free_count;
	
	real_num = pb->entry_count - pb->free_count;
	target_a = (_fpa_target_t*)mem_malloc(MEM_ACLQOS_MODULE, real_num * sizeof(_fpa_target_t));	
	if(target_a)
	{
		return CTC_E_NO_MEMORY;
	}
	memset(target_a, 0, sizeof(real_num * sizeof(_fpa_target_t)));
			
	/*save target idx to array*/
	for(t_idx = 0; t_idx < real_num; t_idx++)
	{
		target_a[t_idx].t_idx = (full_num * t_idx)/real_num;
	}

	/*save old idx to array*/
	o_idx = 0;
	for(idx = 0; t_idx < real_num; t_idx++)
	{
		if(pb->entries[idx])
		{
			target_a[o_idx].o_idx = idx;
			o_idx++;
		}
	}

	left_num = real_num;
	/*move num*/
	while(left_num)
	{
		SYS_ACL_DBG_INFO("");
		
		for(idx = 0; idx < left_num; idx++)
		{
			move_ok = 0;
			if(target_a[idx].o_idx == target_a[idx].t_idx)/* stay */
			{
				SYS_ACL_DBG_INFO("stay\n");
				kal_memmove(&target_a[idx], &target_a[idx+1], (left_num - idx -1)*sizeof(_fpa_target_t));
				left_num--;
				idx--;
			}else{
				if(target-a[idx].o_idx < target_a[idx].t_idx)/* move down */
				{
					if((idx == left_num - 1)||(target_a[idx+1].o_idx > target_a[idx].t_idx))
					{
						move_ok = 1;
					}
				}else{/* move up */
					if((idx == 0)||(target_a[idx-1].o_idx > target_a[idx].t_idx))
					{
						move_ok = 1;
					}
				}

				if(move_ok)
				{
					CTC_ERROR_RETURN(_sys_humber_acl_entry_move(pb->entries[target_a[idx].o_idx], (target_a[idx].t_idx - target_a[idx].o_idx)), ret, cleanup);
					kal_memmove(&target_a[idx], &target_a[idx+1], (left_num - idx -1)*sizeof(_fpa_target_a));
					left_num--;
					idx--;
				}
			}
		}
	}
	mem_free(target_a);

	//end = clock();
	//time = time + (double)end - begin)/CLOCKS_PER_SEC;
	
	return CTC_E_NONE;
cleanup:
	mem_free(target_a);
	return ret;
}
					
static int32
_sys_humber_acl_lookup_block_index(uint16 entry_id, uint16* block_index)
{
	sys_acl_entry_t *pe = NULL;
	*block_index = SYS_ACL_INVALID_INDEX;// SYS_ACL_INVALID_INDEX = -1
	_sys_humber_acl_get_sys_entry_by_eid(entry_id, &pe);
	if(pe)
	{
		*blocK_index = pe->block_index;
	}
	
	return CTC_E_NONE;
}

static int32
_sys_humber_acl_shift_all_entries_down(sys_acl_block_t* pb)
{
	int32 prev_entry_idx = 0;
	int32 left_num;
	int32 idx;
	int32 null_idx = 0;
	
	left_num = pb->entry_count - pb->free_count;
	
	idx = pb->entry_count - 1;
	
	while(left_num)
	{
		if(pb->entries[idx])
		{
			idx--;
			left_num--;
			countinue;
		}

		null_idx = idx;
		
		for(prev_entry_idx = null_idx; prev_entry_idx >= 0; prev_entry_idx--)
		{
			if(pb->entries[prev_entry_idx])
			{
				_sys_humber_acl_entry_move(pb->entries[prev_entry_idx], null_idx - prev_entry_idx);
				break;
			}
		}
		if(prev_entry_idx < 0)
		{
			break;
		}
	}
	return CTC_E_NONE;
}

staitc int32
_sys_humber_acl_shift_all_entries_up(sys_acl_block_t* pb)
{
	int32 next_entry_idx = 0;
	int32 left_num;
	int32 idx;
	int32 null_idx = 0;
	
	left_num = pb->entry_count - pb->free_count;
		
	idx = 0;
	
	while(left_num)
	{
		if(pb->entries[idx])
		{	
			idx++;
			left_num--;
			continue;
		}
		
		null_idx = idx;
		
		for(next_entry_idx = null_idx; next_entry_idx < pb->entry_count; next_entry_idx++)
		{
			if(pb->entries[next_entry_idx])
			{
				_sys_humber_acl_entry_move(pb->entries[next_entry_idx], null_idx - next_enrty_idx);
				break;
			}
		}
		if(next_entry_idx >= pb->entry_count)
		{
			break;
		}
	}
	return CTC_E_NONE;
}

#define SYS_ACL_REMEMBER_BASE 10
#define CTC_ACLQOS_ENTRY_ID_HEAD 0
#define CTC_ACLQOS_ENTRY_ID_TAIL 0xffffffff
/*worst is bset*/
static int32
_sys_humber_acl_magic_reorder(sys_acl_block_t* pb, uint32 after_entry_id)
{
	if((pb->entry_count - pb->free_count) > SYS_ACL_REMEMBER_BASE;//SYS_ACL_REMEMBER_BASE = 10
	{
		return CTC_E_NONE;
	}
	
	if(after_entry_id == CTC_ACLQOS_ENTRY_ID_HEAD)
	{
		pb->after_0_cnt++;
	}
	else if(after_entry_id < CTC_ACLQOS_ENTRY_ID_TAIL)
	{
		pb->after_1_cnt++;
	}
	
	if((pb->entry_count - pb->free_count) == SYS_ACL_REMEMBER_BASE)
	{
		if(pb->after_0_cnt >= 4 + pb->after_1_cnt)
		{
			_sys_humber_acl_shift_all_entries_down(pb);
		}
		else if(pb->after_1_cnt >= 4 + pb->after_0_cnt)	
		{
			_sys_humber_acl_shift_all_entries_up(pb);	
		}
	}
	
	return CTC_E_NONE;		
}

static int32
_sys_humber_acl_get_block_index(sys_acl_block_t *pb, uint32 after_entry_id, uint16* block_index, uint16* shift_amount)
{
	int32 bottom_idx = 0;
	int32 target_idx = 0;
	int32 first_entry_idx = 0;
	int32 idx;
	int32 prev_null_idx = 0;
	int32 next_null_idx = 0;
	int32 shift_up_amount = 0;
	int32 shift_dowm_amounr = 0;
	int16 after_idx = 0;

	CTC_PTR_VALID_CHECK(pb);
	CTC_PTR_VALID_CHECK(block_index);

	if(pb->free_count < 1)
	{
		return CTC_E_ACL_GET_BLOCK_INDEX_FAILED;
	}
	
	*shift_amount = 0;
	
	_sys_humber_acl_magic_reorder(pb, after_entry_id);
	if(after_entry_id == CTC_ACLQOS_ENTRY_ID_TAIL)
	{
		next_null_idx = SYS_ACl_INVALID_INDEX;
		bottom_idx = pb->entry_count + pb->entry_dft_max - 1;
		
		for(idx = pb->entry_count; idx <= bottom_idx; idx++)
		{
			if(pb->entries[idx] == NULL)
			{
				next_null_idx = idx;
				break;
			{
		{
		target_idx = next_null_idx;	
		pb->entry_dft_cnt++;
	}else if(pb->entry_count == pb->free_count){
		target_idx = pb->entry_count - 1;
	}
	else if(after_entry_idx == CTC_ACLQOS_ENTRY_ID_HEAD)
	{
		/*get last not null index*/
		first_entry_index = SYS_ACL_INVALID_INDEX;	
		for( idx = 0; idx < pb->entry_count; idx++)
		{
			if(pb->entries[idx] != NULL)
			{
				first_entry_idx = idx;
				break;
			}
		}
		
		if(first_entry_idx == 0)
		{
			next_null_idx = SYS_ACL_INVALID_INDEX;
			target_idx = 0;
		
			/*search first next null entry from top down*/	
			for(idx = target_idx; idx < pb->entry_count; idx++)
			{
				if(pb->entries[idx] == NULL)
				{
					next_null_idx = idx;
					break;
				}
			}
			
			shift_down_amount = next_null_idx - target_idx - 1;
		
			CTC_ERROR_RETURN(_sys_humber_acl_entry_shift_down(pb, target_idx, next_null_idx));
			*shift_amount = shift_down_amount;
		}eles{
			target_idx = first_entry_idx - 1;
		}
	}else{
		CTC_ERROR_RETURN(_sys_humber_acl_lookup_block_index(after_entry_id, &after_idx));
		next_null_idx = SYS_ACL_INVALID_INDEX;
		prev_null_idx = SYS_ACL_INVALID_INDEX;
		
		for(idx = after_idx; idx >= 0; idx--)
		{
			if(pb->entries[idx] == NULL)
			{
				prev_null_idx = idx;
				break;
			}
		}
		
		for(idx = after_idx; idx < pb->entry_count; idx++)
		{
			if(pb->entries[idx] == NULL)
			{
				next_null_idx = idx;
				break;
			}
		}	

		if(prev_null_idx == SYS_ACL_INVALID_INDEX)
		{
			shift_up_amount = pb->entry_count;
		}else{
			shift_up_amount = after_idx - prev_null_idx;
		}

		if(next_null_idx == SYS_ACL_INVALID_INDEX)
		{
			shift_down_amount = pb->entry_count;
		}else{
			shift_down_amount = next_null_idx - (after_idx + 1);
		}

		if(shift_down_amount <= shift_up_amount)
		{
			/*shift dowm*/	
			target_idx = after_idx + 1;
			*shift_amount = shift_down_amount;
			CTC_ERROR_RETURN(_sys_humber_acl_entry_shift_down(pb, target_idx, prev_null_idx));
		}else{
			/*shift up*/
			target_idx = after_idx;
			*shift_amount = shift_up_amount;
			CTC_ERROR_RETURN(_sys_humber_acl_entry_shift_up(pb, target_idx, next_null_idx)					
		}
	}
	*block_index = target_idx;
	return CTC_E_NONE;
}	
