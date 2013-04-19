#define ETH_ADDR_LEN 6
typedef uint8 mac_addr_t[ETH_ADDR_LEN];

enum aclqos_mac_key_flag_e
{
  ACL_QOS_MAC_KEY_FLAG_MACDA_FLAG  	= 1U << 0,
	ACL_QOS_MAC_KEY_FLAG_MACSA_FLAG  	= 1U << 1,
	ACL_QOS_MAC_KEY_FLAG_COS_FLAG  		= 1U << 2,
	ACL_QOS_MAC_KEY_FLAG_CVLAN_FLAG  	= 1U << 3,
	ACL_QOS_MAC_KEY_FLAG_CTAG_COS_FLAG  	= 1U << 4,
	ACL_QOS_MAC_KEY_FLAG_SVLAN_FLAG  	= 1U << 5,
	ACL_QOS_MAC_KEY_FLAG_STAG_COS_FLAG  	= 1U << 6,
	ACL_QOS_MAC_KEY_FLAG_ETHTYPE_FLAG  	= 1U << 7,
	ACL_QOS_MAC_KEY_FLAG_L2TYPE_FLAG  	= 1U << 8,
	ACL_QOS_MAC_KEY_FLAG_L3TYPE_FLAG  	= 1U << 9,
	ACL_QOS_MAC_KEY_FLAG_CTAG_CFI_FLAG  	= 1U << 10,
	ACL_QOS_MAC_KEY_FLAG_STAG_CFI_FLAG  	= 1U << 11	
};
typedef enum aclqos_mac_key_flag_e aclqos_mac_key_flag_t;


enum aclqos_ipv4_key_flag_e
{
	ACL_QOS_IPV4_KEY_FLAG_MACDA_FLAG  	= 1U << 0,
	ACL_QOS_IPV4_KEY_FLAG_MACSA_FLAG  	= 1U << 1,
	ACL_QOS_IPV4_KEY_FLAG_COS_FLAG  	= 1U << 2,
	ACL_QOS_IPV4_KEY_FLAG_CVLAN_FLAG  	= 1U << 3,
	ACL_QOS_IPV4_KEY_FLAG_CTAG_COS_FLAG  	= 1U << 4,
	ACL_QOS_IPV4_KEY_FLAG_SVLAN_FLAG  	= 1U << 5,
	ACL_QOS_IPV4_KEY_FLAG_STAG_COS_FLAG  	= 1U << 6,
	ACL_QOS_IPV4_KEY_FLAG_L2TYPE_FLAG  	= 1U << 7,
	ACL_QOS_IPV4_KEY_FLAG_L3TYPE_FLAG  	= 1U << 8,

	ACL_QOS_IPV4_KEY_FLAG_IPSA_FLAG  	= 1U << 9,
	ACL_QOS_IPV4_KEY_FLAG_IPDA_FLAG  	= 1U << 10,
	ACL_QOS_IPV4_KEY_FLAG_L4PROTO_FLAG  	= 1U << 11,
	ACL_QOS_IPV4_KEY_FLAG_L4SRCPORT_FLAG  	= 1U << 12,
	ACL_QOS_IPV4_KEY_FLAG_L4DSTPORT_FLAG  	= 1U << 13,
	ACL_QOS_IPV4_KEY_FLAG_TCPFLAG_FLAG  	= 1U << 14,
	ACL_QOS_IPV4_KEY_FLAG_ICMPTYPE_FLAG  	= 1U << 15,
	ACL_QOS_IPV4_KEY_FLAG_ICMPCODE_FLAG  	= 1U << 16,
	ACL_QOS_IPV4_KEY_FLAG_IGMPCODE_FLAG  	= 1U << 17,
	ACL_QOS_IPV4_KEY_FLAG_DSCP_FLAG  	= 1U << 18,
	ACL_QOS_IPV4_KEY_FLAG_PREC_FLAG  	= 1U << 19,
	ACL_QOS_IPV4_KEY_FLAG_PRAG_FLAG  	= 1U << 20,
	ACL_QOS_IPV4_KEY_FLAG_OPTION_FLAG  	= 1U << 21,
	ACL_QOS_IPV4_KEY_FLAG_ROUTEDPKT_FLAG  	= 1U << 22,
	ACL_QOS_IPV4_KEY_FLAG_CTAG_CFI_FLAG  	= 1U << 23,
	ACL_QOS_IPV4_KEY_FLAG_STAG_CFI_FLAG  	= 1U << 24	

};
typedef enum aclqos_ipv4_key_flag_e aclqos_ipv4_key_flag_t;

struct aclqos_mac_key_s
{
	uint32 flags;   /*bitmap of ACLQOS_MAC_KEY_XXX_FLAG*/
	
	mac_addr_t mac_da;
	mac_addr_t mac_da_mask;
	mac_addr_t mac_sa;
	mac_addr_t mac_sa_mask;	
	uitn16 cvlan;
	uint16 cvlan_mask;
	uint16 svlan;
	uint16 svlan_mask;
	uint8 cvlan_mask_valid;
	uint8 svlan_mask_valid;
	uint8 ctag_cos;
	uint8 ctag_cfi;
	uint8 stag_cos;
	uint8 stag_cfi;
	uint8 l2_type;
	uint8 l3_type;
	uint16 eth_type;
	uint16 eth_type;
	uint8 eth_type_mask_valid;
	uint8 cos;
	uint8 rsv[2];
};
typedef struct ctc_aclqos_mac_key_s ctc_aclqos_mac_key_t;


struct aclqos_ipv4_key_s
{
	uint32 flags;   /*bitmap of ACLQOS_IPV4_KEY_XXX_FLAG*/
	
	uint32 ip_sa;
	uint32 ip_sa_mask;
	uint32 ip_da;
	uint32 ip_da_mask;	
	aclqos_l4_port_t l4_src_port;
	aclqos_l4_port_t l4_dst_port;	
	aclqos_ip_frag_t ip_frag;
	aclqos_tcp_flag_t tcp_flag;
	uint8 dscp;
	uint8 l4_protocol;
	uint8 icmp_type;
	uint8 icmp_code;
	uint8 igmp_type;
	uint8 routed_packet;
	uint16 rsv1;

	mac_addr_t mac_da;
	mac_addr_t mac_da_mask;
	mac_addr_t mac_sa;
	mac_addr_t mac_sa_mask;	
	uitn16 cvlan;
	uint16 cvlan_mask;
	uint16 svlan;
	uint16 svlan_mask;
	uint8 cvlan_mask_valid;
	uint8 svlan_mask_valid;
	uint8 ctag_cos;
	uint8 ctag_cfi;
	uint8 stag_cos;
	uint8 stag_cfi;
	uint8 cos;
	uint8 l2_type;
	uint8 l3_type;
	uint8 rsv[2];
};
typedef struct ctc_aclqos_ipv4_key_s ctc_aclqos_ipv4_key_t;
