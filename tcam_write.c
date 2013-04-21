#include "tcam_write.h"

uint16 g_sys_humber_acl_reorder_ratio = 100;
#define DO_REORDER(move_num, all) ((move_num) >= ((all)/g_sys_humber_acl_reorder_ratio))


static sys_aclqos_entry_ctl_t sys_aclqos_entry_ctl;
static ctc_hash_t *p_sys_acl_redirect_hash[MAX_LOCAL_CHIP_NUM];
static sys_aclqos_entry_ctl_t *acl_master = &sys_aclqos_entry_ctl;

static sys_chip_master_t *p_chip_master = NULL;

	
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
	
	CTC_PTR_VALID_CHECK(pe);
	
	asic_type = acl_master->asic_type[pe->key.type];
	pb = &acl_master->block[asic_type];
	if(amount == 0)
	{
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
	int32 shift_down_amount = 0;
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
		next_null_idx = SYS_ACL_INVALID_INDEX;
		bottom_idx = pb->entry_count + pb->entry_dft_max - 1;
		
		for(idx = pb->entry_count; idx <= bottom_idx; idx++)
		{
			if(pb->entries[idx] == NULL)
			{
				next_null_idx = idx;
				break;
			}
		}
		target_idx = next_null_idx;	
		pb->entry_dft_cnt++;
	}else if(pb->entry_count == pb->free_count){
		target_idx = pb->entry_count - 1;
	}
	else if(after_entry_id == CTC_ACLQOS_ENTRY_ID_HEAD)
	{
		/*get last not null index*/
		first_entry_idx = SYS_ACL_INVALID_INDEX;	
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
		}else{
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


void*
ctc_hash_lookup(ctc_hash_t* hash, void *data)
{
	uint32 key = 0;
	uint32 index = 0;
	uint32 hash_size = 0;
	uint16 idx_1d = 0;
	uint16 idx_2d = 0;
	ctc_hash_backet_t *backet = NULL;
	
	if(!hash)
		return NULL;
	
	hash_size = hash->block_size * hash->block_num;
	key = (*hash->hash_key)(data);
	index = key % hash_size;

	idx_1d = index / hash->block_size;
	idx_2d = index % hash->block_size;

	if(!hash->index[idx_1d])
	{
		return NULL;
	}

	for(backet = hash->index[idx_1d][idx_2d]; backet != NULL; backet = backet->next)
	{
		if(backet->key == key && (*hash->hash_cmp)(backet->data, data) == TRUE_
		{
			return backet->data;
		}
	}

	return NULL;
}

int32
sys_humber_aclqos_label_lookup(uint32 lable_id, uint8 is_service_label, sys_aclqos_label_t** pp_label)
{
	sys_aclqos_label_t label;
	
	CTC_NOT_ZERO_CHECK(label_id);
	CTC_PTR_VALID_CHECK(pp_label);
	
	SYS_ACLQOS_LABEL_DBG_FUNC();
	
	label.id = label_id;
		
	if(is_service_label)
	{
		*pp_label = ctc_hash_lookup(p_service_label_hash, &label);
	}else{
		*pp_label = ctc_hash_lookup(p_aclqos_label_hash, &label);	
	}
	
	return CTC_E_NONE;
}

uint8
sys_humber_get_local_chip_num(void)
{
	if( NULL == p_chip_master)
		return 0;
	return p_chip_master->lchip_num;
}

static int32
_sys_humber_aclqos_map_action(uint8 lchip, sys_aclqos_label_t* p_label, ctc_aclqos_action_t *p_ctc_action, sys_aclqos_action_t *p_sys_action)
{
	ctc_direction_t dir;
	sys_nh_offset_array_t offset_array;
	sys_humber_opf_t opf;
	uint32 ds_fwd_offset = 0;
	uint32 ds_fwd_base = 0;
	sys_nh_info_ecmp_t* p_nhinfo = 0;
	sys_acl_redirect_t acl_redirect;
	sys_acl_redirect_t *p_acl_redirect = 0;
	uint32 ret = 0;

	CTC_PTR_VALID_CHECK(pp_label);
	CTC_PTR_VALID_CHECK(p_ctc_action);
	CTC_PTR_VALID_CHECK(p_sys_action);

	SYS_ACLQOS_ENTRY_DBG_FUNC();
	
	kal_memset(&opf, 0, sizeof(opf));
	kal_memset(&ad_redirect, 0, sizeof(acl_redirect));

	dir = p_label->dir;
	
	/*deny*/
	if(p_ctc_action->flag & CTC_ACLQOS_ACTION_DISCARD_FLAG)
	{
		p_sys_action->flag.discard = 1;
	}
	
	//deny bridging
	if(p_ctc_action->flag & CTC_ACLQOS_ACTION_DENY_BRIDGE_FLAG)
	{	
		p_sys_action->flag.deny_bridge = 1;		
	}

	//deny learning
	if(p_ctc_action->flag & CTC_ACLQOS_ACTION_DENY_LEARN_FLAG)
	{	
		p_sys_action->flag.deny_learning = 1;		
	}

	//deny route
	if(p_ctc_action->flag & CTC_ACLQOS_ACTION_DENY_ROUTE_FLAG)
	{	
		p_sys_action->flag.deny_route = 1;		
	}

	//deny replace cos
	if(p_ctc_action->flag & CTC_ACLQOS_ACTION_DENY_REPLACE_COS_FLAG)
	{	
		p_sys_action->flag.deny_replace_cos = 1;		
	}

	//deny replace dscp
	if(p_ctc_action->flag & CTC_ACLQOS_ACTION_DENY_REPLACE_DSCP_FLAG)
	{	
		p_sys_action->flag.deny_replace_dscp = 1;		
	}

	//stats
	if(p_ctc_action->flag & CTC_ACLQOS_ACTION_STATS_FLAG)
	{	
		if(p_ctc_action->flags & CTC_ACLQOS_ACTION_FLOW_ID_FLAGS)
		{
			return CTC_E_ACLQOS_COLLISION_FIELD;
		}
		
		p_sys_action->flag.stats = 1;
		p_sys_action->stats_or_flowid.stats_ptr = SPECIAL_STATS_PTR;
	
		if(p_ctc_action->stats_ptr >= 4096)
		{
			return CTC_E_INVALID_FARAM;
		}
		p_sys_action->stats_or_flowid.stats_ptr = p_ctc_action->stats_ptr;
		SYS_ACLQOS_ENTRY_DBG_INFO();	
	}
	
	//flow id
	if(p_ctc_action->flag & CTC_ACLQOS_ACTION_FLOW_ID_FLAG)
	{	
		CTC_MAX_VALUE_CHECK(p_ctc_action->flow_id, 254);
		p_sys_action->flag.flow_id = 1;		
		
		p_sys_action->stats_or_flowid.flow_id = 255 - p_ctc_action->flow_id;
	}

	//flow policer
	if(p_ctc_action->flag & CTC_ACLQOS_ACTION_FLOW_POLICER_FLAG)
	{	
		p_sys_action->flag.flow_policer = 1;	
		p_sys_action->policer_id = p_ctc_action->policer_id;
	}

	//copy to cpu
	if(p_ctc_action->flag & CTC_ACLQOS_ACTION_COPY_TO_CPU_FLAG)
	{	
		CTC_MAX_VALUE_CHECK(p_ctc_action->log_weight, 0xF);
		p_ctc_action->log_weight = p_ctc_action->log_weight ? p_ctc_action->log_weight : OxF;

		p_sys_action->flag.copy_to_cpu = 1;
		p_sys_action->acl_log_id = CTC_ACLQOS_LOG_SESSION_3;
		p_sys_action->random_threshold_shift = p_ctc_action->log_weight;
	
	}

	//invalid
	if(p_ctc_action->flag & CTC_ACLQOS_ACTION_INVALID_FLAG)
	{	
		p_sys_action->flag.invalid = 1;		
	}

	return CTC_E_NONE;
}


static int32
_sys_humber_aclqos_map_key(uint8 lchip, sys_aclqos_label_t *p_label, ctc_aclqos_key_t *p_ctc_key, sys_aclqos_key_t *p_sys_key)
{
	sys_aclqos_label_index_t *p_label_index;
	

	CTC_PTR_VALID_CHECK(pp_label);
	CTC_PTR_VALID_CHECK(p_ctc_key);
	CTC_PTR_VALID_CHECK(p_sys_key);

	p_label_index = p_label->p_index[lchip];
	CTC_PTR_VALID_CHECK(p_label_index);
	
	p_sys_key->type = p_ctc_key->type;
	
	if(SYS_PBR_ACL_LABEL != p_label->tyoe)
	{
		switch(p_sys_key->type)
		{
			case: CTC_ACLQOS_MAC_KEY:
				CTC_ERROR_RETURN(
					_sys_humber_aclqos_map_mac_key(lchip, p_label, &p_ctc_key->key_info.mac_key, &p_sys_key->key_info.mac_key));
				break;
			case: CTC_ACLQOS_IPV4_KEY:
				CTC_ERROR_RETURN(
					_sys_humber_aclqos_map_ipv4_key(lchip, p_label, &p_ctc_key->key_info.ipv4_key, &p_sys_key->key_info.ipv4_key));
				break;	
			case: CTC_ACLQOS_MPLS_KEY:
				CTC_ERROR_RETURN(
					_sys_humber_aclqos_map_mpls_key(lchip, p_label, &p_ctc_key->key_info.mpls_key, &p_sys_key->key_info.mpls_key));
				break;	
			case: CTC_ACLQOS_IPV6_KEY:
				CTC_ERROR_RETURN(
					_sys_humber_aclqos_map_ipv6_key(lchip, p_label, &p_ctc_key->key_info.ipv6_key, &p_sys_key->key_info.ipv6_key));
				break;	
			default:
				return CTC_E_ACLQOS_INVALID_KEY_TYPE;
		}
	}else{
		/*NULL*/
		return CTC_E_ACLQOS_INVALID_KEY_TYPE;
	}
	
	return CTC_E_NONE;
}

void
ctc_list_pointer_insert_tail(ctc_list_pointer_t *p_list, ctc_list_pointer_node_t *p_node)
{
	p_node->p_next = NULL;
	p_node->p_prev = _CTC_LTAIL(p_list);
	
}


static int32
_sys_humber_aclqos_add_entry_to_db(sys_acl_block_t *pb, sys_aclqos_entry_t *p_entry)
{
	sys_aclqos_label_index_t *p_index = NULL;
	ctc_list_pointer_t *p_list = NULL;
	sys_aclqos_label_t *p_label;
	
	p_label = p_entry->p_label;
	p_index = p_label->p_index[0];
	p_list = &p_index->entry_list[p_entry->key.type];
	ctc_list_pointer_insert_tail(p_list, &p_entry->head);
	
	//add to hash
	ctc_hash_insert(acl_master->entry, p_entry);
	//add to block
	pb->entries[p_entry->block_index] = p_entry;
	
	//free count
	//ignore the default entry
	if(p_entry->block_index < pb->entry_count)
	{
		(pb->free_count)--;
	}

	return CTC_E_NONE;
}

static int32
_sys_humber_aclqos_remove_entry_from_db(sys_acl_block_t *pb, sys_aclqos_entry_t *p_entry)
{
	sys_aclqos_label_index_t *p_index = NULL;
	ctc_list_pointer_t *p_list = NULL;
	sys_aclqos_label_t *p_label;
	sys_humber_opf_t opf;
	uint32 ds_fwd_offset = 0;
	sys_acl_redirect_t acl_redirect;
	sys_acl_redirect_t *p_acL_redirect = 0;
	
	p_label = p_entry->p_label;
	p_index = p_label->p_index[0];
	p_list  = &p_index->entry_list[p_entry->key.type];
	
	//unbind flow policer
	if(p_entry->action.flag.flow_policer)
	{
		sys_humber_qos_flow_policer_unbind(p_sys_acl_redirect_hash[pb->lchip], &acl_redirect);
		if(p_acl_redirect)
		{
			p_acl_redirect->ref--;
		
			if(p_acl_redirect->ref == 0)
			{
				opf.pool_type = OPF_ACL_FWD_SRAM;
				opf.pool_index = pb->lchip;
				ds_fwd_offset = p_entry->action.ds_fwd_ptr;
				CTC_ERROR_RETURN(sys_humber_opf_free_offset9&opf, 1, ds_fwd_offset));
	
				ctc_hash_remove(p_sys_acl_redirect_hash[pb->lchip], p_acl_redirect);
				mem_free(p_acl_redirect);
			}
		}else{
			return CTC_E_ACLQOS_INVALID_ACTION;
		}
	}

	//remove from hash
	ctc_hash_remove(acl_master->entry, p_entry);
	
	//removr from block
	pb->entries[p_entry->block_index] = NULL;
	
	//free count++
	//ignore the default entry
	if(p_entry->block_index < pb->entry_count)
	{
		(pb->free_count)++;
	}
		
	if(p_entry->head.p_prev)
	{
		ctc_list_pointer_delete(p_list, &p_entry->head);
	}
	mem_free(p_entry);
		
	return CTC_E_NONE;
}

static int32
_sys_humber_aclqos_entry_write(sys_acl_block_t *pb, sys_aclqos_entry_t *p_entry)
{
	sys_aclqos_sub_entry_info_t info;
	
	CTC_PTR_VALID_CHECK(p_entry);
	
	memset(&info, 0, sizeof(info));
	info.offset = p_entry->block_index;
	
	CTC_ERROR_RETURN(
		_sys_humber_aclqos_write_entry_to_chip(0, p_entry->p_label, p_entry, &info));
	
	//add to database
	CTC_ERROR_RETURN(
		_sys_humber_aclqos_add_entry_to_db(pb, p_entry));
	
	return CTC_E_NONE;
}

static int32
_sys_humber_aclqos_entry_remove(sys_acl_block_t *pb, sys_aclqos_entry_t* p_entry)
{
	CTC_PTR_VALID_CHECK(p_entry);

	//remove all sub entries from tcam and db
	CTC_ERROR_RETURN(
		_sys_humber_aclqos_remove_sub_entry(0, p_entry->p_label, p_entry);
	
	CTC_ERROR_RETURN(
		_sys_humber_aclqos_remove_entry_from_db(pb, p_entry));
	
	return CTC_E_NONE;	
}


int32
sys_humber_aclqos_entry_insert(uint32 label_id, ctc_aclqos_label_type_t label_type, uint32 entry_id, ctc_aclqos_entry_t *p_ctc_entry)
{
	sys_aclqos_entry_t *p_sys_entry[CTC_MAX_LOCAL_CHIP_NUM];
	sys_aclqos_action_t *p_sys_action;
	sys_aclqos_key_t *p_sys_key;
	sys_aclqos_label_t *p_label = NULL;
	uint8 lchip, lchip_num;
	int32 ret;
	uint8 is_service_label = 0;
	uint16 block_index = 0;
	sys_aclqos_block_t *pb = NULL;
	uint8 asic_type;
	uint16 shift_amount = 0;

	CTC_PTR_VALID_CHECK(p_ctc_entry);
	CTC_MIN_VALUE_CHECK(p_ctc_entry->entry_id, 1);	

	if(CTC_SERVICE_LABEL == label_type)
	{
		is_service_label = 1;
	}

	CTC_ERROE_RETURN(sys_humber_aclqos_label_lookup(label_id, is_service_laebl, &p_label));
	if(!p_label)
	{
		return CTC_E_ACLQOS_LABEL_NOT_EXIST;
	}

	if(CTC_QOS_LABEL == label_type)
	{
		if(IS_ACL_LABEL(p_label->type))
			return CTC_E_ACLQOS_DIFFERENT_TYPE;
	}else if(CTC_ACL_LABEL == label_type)
	{	
		if(IS_QOS_LABEL(p_label->type))
			return CTC_E_ACLQOS_DIFFERENT_TYPE;				
	}

	kal_memset(p_sys_entry, 0, sizeof(p_sys_entry));
	asic_type = acl_master->asic_type[p_ctc_entry->key.type];
	pb = &acl_master->block[asic_type];
	
	//add entry
	lchip_num = sys_humber_get_local_chip_num();
	for(lchip = 0, lchip < lchip_num; lchip++)
	{
		if(!p_label->p_label[lchip])
		{
			continue;
		}
		_sys_humber_acl_get_sys_entry_by_eid(p_ctc_entry->entry_id, &p_sys_entry[lchip]);
		
		if(p_sys_entry[lchip])
		{
			ret = CTC_E_ACLQOS_ENTRY_EXIST;
			p_sys_entry[lchip] = NULL;
			goto ERR;
		}
		
		p_sys_entry[lchip] = (sys_aclqos_entry_t *)mem_malloc(MEM_ACLQOS_MODELE, sizeof(sys_aclqos_entry_t));
		if(!p_sys_entry[lchip])
		{
			ret = CTC_E_NO_MEMORY;
			goto ERR;
		}

		kal_memset(p_sys_entry[lchip], 0, sizeof(sys_aclqos_entry_t));
		p_sys_entry[lchip]->entry_id = p_ctc_entry->entry_id;
		p_sys_action = &p_sys_entry[lchip]->action;
		p_sys_key = &p_sys_entry[lchip]->key;
	
		p_sys_entry[lchip]->p_label = p_label;
	
		//map acl/qos action	
		ret = _sys_humber_aclqos_map_action(lchip, p_label, &p_ctc_entry->action, p_sys_action);
		if(ret)
		{
			goto ERR;
		}

		//map acl/qos key	
		ret = _sys_humber_aclqos_map_key(lchip, p_label, &p_ctc_entry->key, p_sys_key);
		if(ret)
		{
			goto ERR;
		}

		//get entry offset	
		ret = _sys_humber_acl_get_block_index(pb, entry_id, &block_index, &shift_amount);
		if(ret)
		{
			goto ERR;
		}
		p_sys_entry[lchip]->block_index = block_index;
	
		//write entries
		ret = _sys_humber_aclqos_entry_write(pb, p_sys_entry[lchip]);
		if(ret)
		{
			goto ERR;
		}

		if(DO_REORDER(shift_amount, pb->entry_count))
		{
			_sys_humber_acl_reorder(pb, pb->entry_count - 1, 0);
		}
	}
	return CTC_E_NONE;
ERR:
	lchip_num = lchip + 1;
	for(lchip = 0; lchip < lchip_num; lchip++)
	{
		if(!p_label->p_index[lchip])
		{
			continue;
		}
		if(p_sys_entry[lchip])
		{
			_sys_humber_aclqos_entry_remove(pb, p_sys_entry[lchip]);
		}
	}
	
	return ret;
}		

int32
sys_humber_aclqos_entry_delete(uint32 label_id, ctc_aclqos_label_type_t label_type, ctc_aclqos_key_type_t entry_type, uint32 entry_id)
{
	sys_aclqos_label_t *p_label;
	sys_aclqos_entry_t *p_entry;
	uint8 lchip, lchip_num;
	uint8 is_service_label = 0;
	uint8 asic_type;
	sys_acl_block_t *pb = NULL;
	
	if(CTC_SERVICE_LABEL == label_type)
	{
		is_service_label = 1;
	}

	CTC_ERROR_RETURN(sys_humber_aclqos_label_lookup(label_id, is_service_label, &p_label));
	if(!p_label)
	{
		return CTC_E_ACLQOS_LABEL_NOT_EXIST);
	}

	if(CTC_QOS_LABEL == label_type)
	{
		if(IS_ACL_LABEL(p_label->type))
			return CTC_E_ACLQOS_DIFFERENT_TYPE;
	}else if(CTC_ACL_LABEL == label_type)
	{	
		if(IS_QOS_LABEL(p_label->type))
			return CTC_E_ACLQOS_DIFFERENT_TYPE;				
	}
	
	lchip_num = sys_humber_get_local_chip_num();
	for(lchip = 0; lchip < lchip_num; lchip++)
	{
		if(!p_label->p_index[lchip])
		{
			continue;
		}
		
		_sys_humber_acl_get_sys_entry_by_eid(entry_id, &p_entry);
		if(!p_entry)
		{
			return CTC_E_ACLQOS_ENTRY_NOT_EXIST;
		}

	}

	asic_type = acl_master->asic_type[p_entry->key.type];
	pb = &acl_master->block[asic_type];

	CTC_ERROR_RETURN(_sys_humber_aclqos_entry_remove(pb, p_entry));
	if(pb->free_count == pb->entry_count)
	{
		pb->after_0_cnt = 0;
		pb->after_1_cnt = 0;
	}

	return CTC_E_NONE;
}

int32
sys_humber_aclqos_entry_delete_all(uint32 label_id, ctc_aclqos_label_type_t label_type, ctc_aclqos_key_type_t entry_type)
{
	sys_aclqos_label_t *p_label;
	ctc_list_pointer_t *p_list;
	ctc_list_pointer_node_t *p_node, *p_next_node;
	sys_aclqos_entry_t *p_entry = NULL;
	uint8 lchip, lchip_num;
	uint8 is_service_label = 0;
	uint8 asic_type;
	sys_acl_block_t *pb = NULL;
	
	if(CTC_SERVICE_LABEL == label_type)
	{
		is_service_label = 1;
	}

	CTC_ERROR_RETURN(sys_humber_aclqos_label_lookup(label_id, is_service_label, &p_label));
	if(!p_label)
	{
		return CTC_E_ACLQOS_LABEL_NOT_EXIST);
	}	

	if(CTC_QOS_LABEL == label_type)
	{
		if(IS_ACL_LABEL(p_label->type))
			return CTC_E_ACLQOS_DIFFERENT_TYPE;
	}else if(CTC_ACL_LABEL == label_type)
	{	
		if(IS_QOS_LABEL(p_label->type))
			return CTC_E_ACLQOS_DIFFERENT_TYPE;				
	}


	asic_type = acl_master->asic_type[p_entry->key.type];
	pb = &acl_master->block[asic_type];

	lchip_num = sys_humber_get_local_chip_num();
	for(lchip = 0; lchip < lchip_num; lchip++)
	{
		if(!p_label->p_index[lchip])
		{
			continue;
		}

		p_list = &(p_label->p_index[lchip]->entry_list[entry_type]);
		CTC_LIST_POINTER_LOOP_DEL(p_node, p_next_node, p_list)
		{
			p_entry = _ctc_container_of(p_node, sys_aclqos_entry_t, head);
	
			CTC_ERROR_RETURN(_sys_humber_aclqos_entry_remove(pb, p_entry));			
		}
	}
	return CTC_E_NONE;
}

int32
sys_humber_aclqos_entry_action_add(uint32 label_id, ctc_aclqos_label_type_t label_type, ctc_aclqos_key_type_t entry_type, uint32 entry_id, ctc_aclqos_action_t *p_action)
{
	sys_aclqos_label_t *p_label;
	sys_aclqos_entry_t *p_entry;
	uint8 lchip, lchip_num;
	uint32 cmd, tmp;
	tbl_id_t action_tbl_id;
	fld_id_t action_fld_id;
	ctc_list_pointer_t *p_list;
	ctc_list_pointer_node_t *p_node;
	sys_aclqos_sub_entry_info_t *p_info;
	uint8 is_service_label = 0;
	sys_nh_offset_array_t offset_array;
	uint16 pbr_vrfId = 0;
	uint32 policer_ptr = 0;
	uint32 ds_fwd_offset = 0;
	uint32 ds_fwd_base = 0;
	uint32 ds_fwd_offset_old = 0;
	uint32 nhid_old = 0;
	sys_aclqos_redirect_t acl_redirect;
	sys_aclqos_redirect_t *p_acl_redirect = 0;
	sys_humber_opf_t opf;
	int32 ret = 0;
	
	kal_memset(&acl_redirect, 0, sizeof(acl_redirect));
	kal_memeset(&opf, 0, sizeof(opf));
	
	CTC_PTR_VALID_CHECK(p_action);

	if(CTC_SERVICE_LABEL == label_type)
	{
		is_service_label = 1;
	}

	CTC_ERROR_RETURN(sys_humber_aclqos_label_lookup(label_id, is_service_label, &p_label));
	if(!p_label)
	{
		return CTC_E_ACLQOS_LABEL_NOT_EXIST;
	}

	if(CTC_QOS_LABEL == label_type)
	{
		if(IS_ACL_LABEL(p_label->type))
			return CTC_E_ACLQOS_DIFFERENT_TYPE;
	}else if(CTC_ACL_LABEL == label_type){	
		if(IS_QOS_LABEL(p_label->type))
			return CTC_E_ACLQOS_DIFFERENT_TYPE;				
	}

	lchip_num = sys_humber_get_local_chip_num();
	for(lchip = 0; lchip < lchip_num; lchip++)
	{
		if(!p_label->p_index[lchip])
		{
			continue;
		}
		
		_sys_humber_acl_get_sys_entry_by_eid(entry_id, &p_entry);
		if(!p_entry)
		{
			return CTC_E_ACLQOS_ENTRY_NOT_EXIST;
		}

		if(p_action->flag & CTC_ACLQOS_ACTION_FLOW_POLICER_FLAG)
		{
			if(p_entry->action.flag.flow_policer)
			{
				sys_humber_qos_flow_policer_unbind(lchip, p_entry_action.policer_id);
			}

			//bind flow policer to chip
			CTC_ERROR_RETURN(sys_humber_qos_flow_policer_bind(lchip, p_action->policer_id));	
			CTC_ERROR_RETURN(sus_humber_qos_policer_index_get(lchip, p_action->policer_id, &policer_ptr));
		
			//write to chip
		}
	}
}


