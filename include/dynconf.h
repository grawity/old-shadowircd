#ifndef	__dynconf_include__
#define __dynconf_include__ 1

#define DYNCONF_H

/* config level */
#define DYNCONF_CONF_VERSION "1.1.3"
#define DYNCONF_NETWORK_VERSION "2.1.7"

typedef struct zNetwork aNetwork;
struct zNetwork
{

  char *x_ircnetwork;
  char *x_ircnetwork_name;
  char *x_defserv;
  char *x_website;
  char *x_rules_page;
  char *x_network_kline_address;
  unsigned x_mode_x:1;
  unsigned x_mode_x_lock:1;
  unsigned x_clone_protection:1;
  long x_clone_limit;
  unsigned x_oper_clone_limit_bypass:1;
  unsigned x_hideulinedservs:1;
  unsigned x_global_connect_notices:1;
  unsigned x_oper_autoprotect:1;
  long x_oper_mode_override;
  unsigned x_oper_mode_override_notice:1;
  char *x_users_auto_join;
  char *x_opers_auto_join;
  char *x_services_server;
  char *x_statserv_server;
  unsigned x_enable_nickserv_alias;
  char *x_nickserv_name;
  unsigned x_enable_chanserv_alias;
  char *x_chanserv_name;
  unsigned x_enable_memoserv_alias;
  char *x_memoserv_name;
  unsigned x_enable_botserv_alias;
  char *x_botserv_name;
  unsigned x_enable_rootserv_alias;
  char *x_rootserv_name;
  unsigned x_enable_operserv_alias;
  char *x_operserv_name;
  unsigned x_enable_statserv_alias;
  char *x_statserv_name;
  char *x_operhost;
  char *x_helpchan;


};

typedef struct zConfiguration aConfiguration;
struct zConfiguration
{
  unsigned hub:1;
  long client_flood;
  long max_channels_per_user;
  char *server_kline_address;
  unsigned def_mode_i:1;
  unsigned short_motd:1;
  char *include;
  aNetwork network;
};

#ifndef DYNCONF_C
extern aConfiguration iConf;
#endif

#define IRCNETWORK			iConf.network.x_ircnetwork
#define IRCNETWORK_NAME			iConf.network.x_ircnetwork_name
#define DEFSERV				iConf.network.x_defserv
#define WEBSITE				iConf.network.x_website
#define RULES_PAGE			iConf.network.x_rules_page
#define NETWORK_KLINE_ADDRESS		iConf.network.x_network_kline_address
#define MODE_x				iConf.network.x_mode_x
#define MODE_x_LOCK			iConf.network.x_mode_x_lock
#define CLONE_PROTECTION		iConf.network.x_clone_protection
#define CLONE_LIMIT			iConf.network.x_clone_limit
#define OPER_CLONE_LIMIT_BYPASS		iConf.network.x_oper_clone_limit_bypass
#define HIDEULINEDSERVS			iConf.network.x_hideulinedservs
#define GLOBAL_CONNECTS			iConf.network.x_global_connect_notices
#define OPER_AUTOPROTECT		iConf.network.x_oper_autoprotect
#define OPER_MODE_OVERRIDE		iConf.network.x_oper_mode_override
#define OPER_MODE_OVERRIDE_NOTICE	iConf.network.x_oper_mode_override_notice
#define USERS_AUTO_JOIN			iConf.network.x_users_auto_join
#define OPERS_AUTO_JOIN			iConf.network.x_opers_auto_join
#define SERVICES_SERVER			iConf.network.x_services_server
#define STATSERV_SERVER			iConf.network.x_statserv_server
#define ENABLE_NICKSERV_ALIAS		iConf.network.x_enable_nickserv_alias
#define NICKSERV_NAME			iConf.network.x_nickserv_name
#define ENABLE_CHANSERV_ALIAS		iConf.network.x_enable_chanserv_alias
#define CHANSERV_NAME			iConf.network.x_chanserv_name
#define ENABLE_MEMOSERV_ALIAS		iConf.network.x_enable_memoserv_alias
#define MEMOSERV_NAME			iConf.network.x_memoserv_name
#define ENABLE_BOTSERV_ALIAS		iConf.network.x_enable_botserv_alias
#define BOTSERV_NAME			iConf.network.x_botserv_name
#define ENABLE_ROOTSERV_ALIAS		iConf.network.x_enable_rootserv_alias
#define ROOTSERV_NAME			iConf.network.x_rootserv_name
#define ENABLE_OPERSERV_ALIAS		iConf.network.x_enable_operserv_alias
#define OPERSERV_NAME			iConf.network.x_operserv_name
#define ENABLE_STATSERV_ALIAS		iConf.network.x_enable_statserv_alias
#define STATSERV_NAME			iConf.network.x_statserv_name
#define OPERHOST			iConf.network.x_operhost
#define HELPCHAN			iConf.network.x_helpchan

#define HUB				iConf.hub
#define CLIENT_FLOOD			iConf.client_flood
#define MAXCHANNELSPERUSER		iConf.max_channels_per_user
#define SERVER_KLINE_ADDRESS		iConf.server_kline_address
#define DEF_MODE_i			iConf.def_mode_i
#define SHORT_MOTD			iConf.short_motd
#define INCLUDE				iConf.include

#endif /* __dynconf_include__ */
