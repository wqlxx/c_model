

struct cm_hash_backet_s
{
	struct cm_hash_backet_s *next;
	
	uint32 key;
	
	void *data;
};
typedef struct cm_hash_backet_s cm_hash_backet_t;


struct cm_hash_s
{
	cm_hash_backet_t ***index;

	/* <Hash table size = block_size * block_num*/
	
	/* Hash table block num*/
	uint16 block_num;

	/* Hash table block size*/	
	uint16 block_size;

	/*current hash backet size*/
	uint32 count;
	
	/* key make function*/
	uint32 (*hash_key)(void* data);

	/*data compare function*/
	bool (*hash_cmp)(void* backet_data, void* data);
};
typedef struct cm_hash_s cm_hash_t;

