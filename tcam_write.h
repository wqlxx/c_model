struct sys_aclqos_flag_s
{
  uint32 discard:1,
		deny_replace_cos:1,
		deny_replace_dscp:1,
		deny_bridge:1,
		deny_learning:1,
		deny_route;1,
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
}
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
}
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
		svlan;1,
		stag_cos:1,
		stag_cfi:1,
		eth_type:1,
		l2_type:1,
		l3_type:1,
		acl_type:1,
		qos_type:1,
		l2_qos_label:1,
		l3_qos_label:1,
		is_glb_entry;1,
		rsv:14;
}
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
}
typedef struct sys_aclqos_mac_key_s sys_aclqos_mac_key_t;

struct sys_aclqos_ipv4_key_flag_s
{
	uint32 ipda;1,
		ipsa:1,
		l4info_mapped;1,
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
}
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
}
typedef struct sys_aclqos_ipv4_key_s sys_aclqos_ipv4_key_t;
