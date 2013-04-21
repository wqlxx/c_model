#define SYS_ACL_REMEMBER_BASE 10
#define SYS_ACL_INVALID_INDEX -1
#define CTC_ACLQOS_ENTRY_ID_HEAD 0
#define CTC_ACLQOS_ENTRY_ID_TAIL 0xffffffff


struct sys_chip_master_s
{
	uint8 lchip_num;
	uint8 resv;
	uint8 g_chip_id[CTC_MAX_LOCAL_CHIP_NUM];
};
typedef struct sys_chip_master_s sys_chip_master_t;

typedef struct
{
	uint16 t_idx;//target index
	uint16 o_idx;//old index
}_fpa_target_t;


struct sys_aclqos_flag_s
{
	uint32 discard:1,
		deny_replace_cos:1,
		deny_replace_dscp:1,
		deny_bridge:1,
		deny_learning:1,
		deny_route:1,
		stats:1,
		flow_policer:1,
		trust:1,
		priority:1,
		random_log:1,
		fwd:1,
		fwd_to_cpu:1,
		flow_id:1,
		stats_mode:1,
		invalid:1,
		pbr_fwd:1,
		pbr_ttl_check:1,
		pbr_icmp_check:1,
		pbr_ecmp:1,
		pbr_copy_to_cpu:1,
		pbr_deny:1,
		rsv:10;
};
typedef struct sys_aclqos_flag_s sys_aclqos_flag_t;

struct sys_aclqos_action_s
{
	sys_aclqos_action_flag_t flag;

	union
	{
		uint32 fwd_nh_id;
		uint32 fwd_reason;
	}fwd;

	union
	{
		uint16 stats_ptr;	
		uint16 flow_id;
	}stats_or_flowid;
	uint16 rsv1;

	uint32 policer_id;
	
	uint32 priority:6,
		color:2,
		trust:3,	
		acl_log_id:2,
		random_threshold_shift:4,
		ds_fwd_ptr:12,
		rsv2:15;

	uint32 pbr_fwd_ptr :20,
		rsv:12;
	uint16 pbr_vrfid;
	uint8 pbr_ecpn;
	uint8 rsv4;
};
typedef struct sys_aclqos_action_s sys_aclqos_action_t;

struct sys_aclqos_mac_key_flag_s
{
	uint32 macda:1,
		macsa:1,
		vlan_ptr:1,
		cos:1,
		cvlan:1,
		ctag_cos:1,
		ctag_cfi:1,
		svlan:1,
		stag_cos:1,
		stag_cfi:1,
		eth_type:1,
		l2_type:1,
		l3_type:1,
		acl_type:1,
		qos_type:1,
		l2_qos_label:1,
		l3_qos_label:1,
		is_glb_entry:1,
		rsv:14;
};
typedef struct sys_aclqos_mac_key_flag_s sys_aclqos_mac_key_flag_t;

#define CTC_ETH_ADDR_LEN  6
typedef uint8 mac_addr_t[CTC_ETH_ADDR_LEN];//CTC_ETH_ADDR_LEN = 6

struct sys_aclqos_mac_key_s
{
	sys_aclqos_mac_key_flag_t flag;
	
	mac_addr_t mac_da;
	mac_addr_t mac_da_mask;
	mac_addr_t mac_sa;
	mac_addr_t mac_sa_mask;
	uint32 vlan_ptr:14,
		eth_type:16,
		rsv1:2;
	uint32 cvlan:12,
		ctag_cos_cfi:4,
		svlan:12,
		stag_cos_cfi:4;
	uint32 cvlan_mask:12,
		svlan_mask:12,
		rsv2:8;
	uint32 l2_type:4,
		l3_type:4,
		cos:3,
		rsv3:21;
	uint32 acl_label:8,
		qos_label:8,
		l2_qos_label:8,
		l3_qos_label:8;
	uint32 eth_type_mask:8,
		table_id0:4,
		table_id1:4,
		table_id2:4,
		table_id3:4;							
};
typedef struct sys_aclqos_mac_key_s sys_aclqos_mac_key_t;

struct sys_aclqos_ipv4_key_flag_s
{
	uint32 ipda:1,
		ipsa:1,
		l4info_mapped:1,
		is_application:1,
		is_tcp:1,
		is_udp:1,
		l4_src_port:1,
		l4_dst_port:1,
		tcp_flag:1,
		dscp:1,
		frag_info:1,
		ip_option:1,
		routed_packet:1,
		macda:1,
		macsa:1,
		cos:1,
		cvlan:1,
		ctag_cos:1,
		ctag_cfi:1,
		svlan:1,
		stag_cos:1,
		stag_cfi:1,		
		l2_type:1,
		l3_type:1,
		acl_label:1,
		qos_label:1,
		l2_qos_label:1,
		l3_qos_label:1,
		is_glb_entry:1,
		rsv:2;	
};
typedef struct sys_aclqos_ipv4_key_flag_s sys_aclqos_ipv4_key_flag_t;

struct sys_aclqos_ipv4_key_s
{
	sys_aclqos_ipv4_key_flag_t flag;
	
	uint32 ip_sa;
	uint32 ip_sa_mask;
	uint32 ip_da;
	uint32 ip_da_mask;
	uint16 l4_src_port;
	uint16 l4_src_port_mask;
	uint16 l4_dst_port;
	uint16 l4_dst_port_mask;
	uint16 l4info_mapped;
	uint16 l4info_mapped_mask;
	uint32 frag_info:2,
		frag_info_mask:2,
		dscp:6,
		dcsp_mask:6,
		is_application:1,
		is_tcp:1,
		is_udp:1,
		routed_packet:1,	
		ip_pbr_error:1,
		rsv1:11;

	mac_addr_t mac_da;
	mac_addr_t mac_da_mask;
	mac_addr_t mac_sa;
	mac_addr_t mac_sa_mask;
	uint32 cvlan:12,
		ctag_cos_cfi:4,
		svlan:12,
		stag_cos_cfi:4;
	uint32 cvlan_mask:12,
		svlan_mask:12,
		rsv4:8;	
	uint32 cos:3,
		l2_type:4,
		l3_type:4,
		rsv2:21;

	uint32 acl_label:8,
		qos_label:8,
		l2_qos_label:8,
		l3_qos_label:8;

	uint32 acl_label_mask:8,
		qos_label_mask:8,
		l2_qos_label_mask:8,
		l3_qos_label_mask:8;		

	uint32 	table_id0:4,
		table_id1:4,
		table_id2:4,
		table_id3:4,
		rsv3;16;		
};
typedef struct sys_aclqos_ipv4_key_s sys_aclqos_ipv4_key_t;

struct sys_aclqos_sub_entry_info_flag_s
{
	uint32 l4_src_port:1,
		l4_dst_port:1,
		frag_info:1,
		ext_hdrL:1,
		reset_l4info:1,
		rsv:27;
};
typedef struct sys_aclqos_sub_entry_info_flag_s sys_aclqos_sub_entry_info_flag_t;

struct sys_aclqos_sub_entry_info_s
{
	sys_aclqos_sub_entry_info_flag_t flag;
	
	uint16 l4_src_port;
	uint16 l4_src_port_mask;
	uint16 l4_dst_port;
	uint16 l4_dst_port_mask;
	uint8 frag_info;
	uint8 frag_info_mask;
	uint8 ext_hdr;
	uint8 ext_hdr_mask;

	uint32 offset;
};
typedef struct sys_aclqos_sub_entry_info_s sys_aclqos_sub_entry_info_t;

enum ctc_aclqos_key_type_e
{
	CTC_ACLQOS_MAC_KEY = 0,
	CTC_ACLQOS_IPV4_KEY
};
typedef enum ctc_aclqos_key_type_e ctc_aclqos_key_type_t;

struct sys_aclqos_key_s
{
	ctc_aclqos_key_type_t type;

	union
	{
		sys_aclqos_mac_key_t mac_key;
		sys_aclqos_ipv4_key_t ipv4_key;
	}key_info;
};
typedef struct sys_aclqos_key_s sys_aclqos_key_t;


typedef struct ctc_list_pointer_node_s
{
	struct ctc_list_pointer_node_s* p_next;
	struct ctc_list_pointer_node_s* p_prev;
}ctc_list_pointer_node_t; 

enum sys_aclqos_label_type_e
{
	SYS_PORT_ACL_LABEL,
	SYS_VLAN_ACL_LABEL,
	SYS_PBR_ACL_LABEL,
	SYS_PORT_QOS_LABEL,
	SYS_VLAN_QOS_LABEL,
	SYS_SERVICE_ACLQOS_LABEL,
	
	MAX_SYS_ACLQOS_LABEL
};
typedef enum sys_aclqos_label_type_e sys_aclqos_label_type_t;

struct sys_aclqos_label_s
{
	uint32 id;
	sys_aclqos_label_index_t *p_index[CTC_MAX_LOCAL_CHIP_NUM];// CTC_MAX_LOCAL_CHIP_NUM = 2
	
	uint8 type;/* SYS_XXX_LABEL*/
	uint8 dir;
	uint8 ref;
};
typedef struct sys_aclqos_label_s sys_aclqos_label_t;

struct sys_aclqos_entry_s
{
	ctc_list_pointer_node_t head;//double direction link

	uint32 entry_id;
	
	sys_aclqos_action_t action;
	sys_aclqos_key_t key;
	
	uint16 block_index;
	void *p_label;/*sys_aclqos_label_t*/
};
typedef struct sys_aclqos_entry_s sys_aclqos_entry_t;
typedef struct sys_aclqos_entry_s sys_acl_entry_t;

struct sys_aclqos_global_entryid_list_s
{
	ctc_list_pointer_node_t head;
	sys_aclqos_entry_t *p_entry;
};
typedef struct sys_aclqos_global_entryid_list_s sys_aclqos_global_entryid_list_t;

struct sys_acl_block_s
{
	uint8 block_number;/*physical block 0~48?
	uint8 block_type;/*mac/ipv4*/
	uint16 entry_count;/*entry count on each block, 512, uint16 is enough*/
	uint16 entry_dft_cnt;
	uint16 entry_dft_max;
	uint16 free_count;
	sys_aclqos_entry_t **entries;
	uint8 lchip;
	uint16 after_0_cnt;//entry count < SYS_REMEMBER_BASE
	uint16 after_1_cnt;
};
typedef struct sys_acl_block_s sys_acl_block_t;
	
#define SYS_ACL_ASIC_TYPE_MAX 2

struct ctc_hash_backet_s
{
	struct ctc_hash_backet_s *next;
	
	uint32 key;
	
	void *data;
};
typedef struct ctc_hash_backet_s ctc_hash_backet_t;

struct ctc_hash_s
{
	ctc_hash_backet_t ***index;

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
typedef struct ctc_hash_s ctc_hash_t;

struct sys_aclqos_entry_ctl_s
{
	uint8 entry_sort_mode;
	uint8 is_merge_mac_ip_key;
	uint8 is_dual_aclqos_lookup;
	uint8 disable_merge_mac_ip_key_physical;
	uint16 mac_ipv4_acl_entry_num;
	uint16 ipv6_acl_entry_num;
	uint16 mac_ipv4_acl_entry_num;
	uint32 acl_fwd_base;
	uint32 global_aclqos_entry_head_num;
	uint32 global_aclqos_entry_tail_num;
	
	ctc_hash_t* entry;
	sys_acl_block_t block[SYS_ACL_ASIC_TYPE_MAX];// SYS_ACL_ASIC_TYPE_MAX = 2
	uint8 asic_type[MAX_CTC_ACLQOS_KEY];
};
typedef struct sys_aclqos_entry_ctl_s sys_aclqos_entry_ctl_t;	
