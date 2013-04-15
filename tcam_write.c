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

/*move entry in hardware table to an new indec8?
static int32
_sys_humber_acl_entry_move_hw(sys_acl_entry_t *pe, int32 tcam_idx_new)
{
	int32 tcam_idx_old = pe->block_index;
	sys_aclqos_sub_entry_info_t sub_info;
	
	CTC_PTR_VALID_CHECK(pe);
	SYS_ACL_DBG_FUNC();
	
	/*add first*/
	memset(&sub_info, 0, sizeof(sub_info));
}
