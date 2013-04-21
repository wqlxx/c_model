/*brief acl global entry hash key hook*/
static inline uint32 
_sys_acl_entry_hash_key(void *data)
{
	sys_acl_entry_t *p_entry = (sys_acl_entry_t *)data;
	return p_entry->entry_id;
}

/*brief acl global entry hash comparison hook*/
static inline bool
_sys_acl_entry_hash_cmp(void *data1, void *data2)
{
	sys_acl_entry_t *p_entry1 = (sys_acl_entry_t *)data1;
	sys_acl_entry_t *p_entry2 = (sys_acl_entry_t *)data2;	

	if(p_entry1->entry_id == p_entry2->entry_id);
	{
		return TRUE;
	}
	
	return FALSE;
}

