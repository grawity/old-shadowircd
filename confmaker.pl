#!/usr/bin/perl
# $Id: confmaker.pl,v 3.4 2004/09/22 17:54:30 nenolod Exp $
# ShadowIRCd ircd.conf maker - (c) 2004 NhJm

my $cfile;
sub in {
my ($r,$d) = @_;
my $o = 1 if (!$d);
start:
print "$r [$d] ";
my $t = <STDIN>; chomp $t;
goto start if ($o && !$t);
return $d if (!$o && !$t);
return $t;
}
sub a { $cfile .= join("", @_)."\n"; }
sub b { ($n,$b,$l) = @_; &a("    $n = \"", &in($b,$l), "\";"); print "\n"; }


&a("configuration {\n  serverinfo {");

print "\nServer Information\n\n";

&b("name","Server Name","server.example.com");
&b("sid","A server's unique ID. This is three characters long and must be in the form [0-9][A-Z/0-9][A-Z/0-9]. The first character must be a digit, followed by 2 alpha-numerical letters. Note: The letters must be capitalized.","666");
&b("description","Server Description. Don't use '[' or ']'.","shadowircd-powered server of doom!");
&b("hub","Will this server be a hub? (yes/no)","no");
&a("    /* vhost = \"192.169.0.1\"; */");
&a("    /* vhost6 = \"3ffe:80e8:546::2\"; */");
&b("max_clients","Maximum amount of clients allowed to connect","512");
&a("    /* These are for SSL. If you want to make the files run this
     * command:
     *
     *          openssl req -new -x509 -days 365 -nodes \
     *                  -out \"<config dir>/cert.pem\" \
     *                  -keyout \"<config dir>/rsa.key\" 
     *          openssl x509 -subject -dates -fingerprint -noout \
     *                  -in \"$<config dir>/cert.pem\"
     *
     * ...where <config dir> is the path of your configuration
     * directory.
     */

    /* rsa key: the path to the file containing our rsa key for cryptlink.
     *
     * Example command to store a 2048 bit RSA keypair in
     * rsa.key, and the public key in rsa.pub:
     */
    /* rsa_private_key_file = \"/usr/local/ircd/etc/rsa.key\"; */

    /* ssl certificate
     *
     * Provided you have made a private key for cryptlinks, you
     * can support SSL client connections by generating an SSL
     * certificate file.
     *
     */
    /* ssl_certificate_file = \"/usr/local/ircd/etc/cert.pem\"; */

    /* certification authority certificate
     *
     * if you did not sign the certificate yourself, you will need
     * to provide the path to the CA's certificate here.
     */
    /* ssl_ca_certificate_file = \"/usr/local/ircd/etc/ca.pem\"; */");

&a("  };\n\n  network {");

print "\n\nNetwork Information\n\n";

&b("name","Network Name?","MooNet");
&b("description","Network description?","A nice place for cows to graze.");
&b("on_oper_host","Virtual host opers will recieve when the oper up.","staff.moonet.com");
&b("cloak_on_oper","Apply staff cloak on oper? (yes/no)","yes");
&b("cloak_on_connect","Apply the standard usercloak upon connect.","yes");
&b("cloak_key_1","Cloak key #1. (sets the cloak keys for the +v usercloaking feature). Alphanumeric string.","aeftueqakvhx127");
&b("cloak_key_2","Cloak key #2.","ackckfriewiaz781");
&b("cloak_key_3","Cloak key #3.","eieir585481217f2");
&b("gline_address","Address to get support if user finds himself G:Lined","some.website.or.email.here");

&a("  };\n\n  admin {");

print "\n\nAdmin Information\n\n";

&b("name","Administrator Name","Smurf Target");
&b("description","Administrator Description","Main Server Administrator");
&b("email","Administrator's E-Mail:","<syn\@packets.r.us>");

&a("  };\n\n  logging {");

print "\n\nLogging\n\n";

&b("use_logging","Do you want to enable logging?","yes");
&a("    fuserlog = \"logs/userlog\";
    foperlog = \"logs/operlog\";
    ffailed_operlog = \"logs/foperlog\";

    /* log level: the amount of detail to log in ircd.log.  The
     * higher, the more information is logged.  May be changed
     * once the server is running via /quote SET LOG.  Either:
     * L_CRIT, L_ERROR, L_WARN, L_NOTICE, L_TRACE, L_INFO or L_DEBUG
     */
    log_level = L_INFO;
  };
  
  class {
    /* name: the name of the class.  classes are text now */
    name = \"users\";

    /* ping time: how often a client must reply to a PING from the
     * server before they are dropped.
     */
    ping_time = 2 minutes;

    /* number per ip: the number of users per host allowed to connect */
    number_per_ip = 2;

    /* max number: the maximum number of users allowed in this class */
    max_number = 100;

    /* sendq: the amount of data allowed in a clients queue before
     * they are dropped.
     */
    sendq = 100 kbytes;
  };

  class {
    name = \"restricted\";
    ping_time = 1 minute 30 seconds;
    number_per_ip = 1;
    max_number = 100;
    sendq = 60 kb;
  };

  class {
    name = \"opers\";
    ping_time = 5 minutes;
    number_per_ip = 10;
    max_number = 100;
    sendq = 100 kbytes;
  };

  class {
    name = \"server\";
    ping_time = 5 minutes;

    /* connectfreq: only used in server classes.  specifies the delay
     * between autoconnecting to servers.
     */
    connectfreq = 5 minutes;

    /* max number: the amount of servers to autoconnect to */
    max_number = 1;

    /* sendq: servers need a higher sendq as they send more data */
    sendq = 2 megabytes;
  };
  
  listen {
    /* port: the specific port to listen on.  if no host is specified
     * before, it will listen on all available IPs.
     *
     * ports are separated via a comma, a range may be specified using ".."
     */

    /* port: listen on all available IPs, ports 6665 to 6669 */
    port = 6665 .. 6669;

    /* sslport: listen on all available IPs, ports 9997 to 9999 for ssl */
    sslport = 9997 .. 9999;

    /* host: set a specific IP/host the ports after the line will listen 
     * on.  This may be ipv4 or ipv6.
     */
    /* Examples:
    host = \"1.2.3.4\";
    port = 7000, 7001;

    host = \"3ffe:1234:a:b:c::d\";
    port = 7002; */
  };
  
  auth {
    user = \"*@*\";
    class = \"users\";
  };

/* deny {}: IPs that are not allowed to connect (before DNS/ident lookup)
 * Oper issued dlines will be added to the specified dline config
 */
  deny {
    ip = \"10.0.1.0/24\";
    reason = \"Reconnecting vhosted bots\";
  };

/* exempt {}: IPs that are exempt from deny {} and Dlines. (OLD d:) */
  exempt {
    ip = \"192.168.0.0/16\";
  };

/* resv {}: nicks and channels users may not use/join (OLD Q:) */
  resv {
    /* reason: the reason for the proceeding resv's */
    reason = \"There are no services on this network\";

    /* resv: the nicks and channels users may not join/use */
    nick = \"nickserv\";
    nick = \"chanserv\";
    channel = \"#services\";

    /* resv: wildcard masks are also supported in nicks only */
    reason = \"Clone bots\";
    nick = \"clone*\";
  };

/* gecos {}:  The X: replacement, used for banning users based on their
 * \"realname\".  The action may be either:
 *      warn:   allow client to connect, but send message to opers
 *      reject: drop clients but also send message to opers.
 *      silent: silently drop clients who match.
 */
  gecos {
    name = \"*sex*\";
    reason = \"Possible spambot\";
    action = warn;
  };

  gecos {
    name = \"sub7server\";
    reason = \"Trojan drone\";
    action = reject;
  };

  gecos {
    name = \"*http*\";
    reason = \"Spambot\";
    action = silent;
  };

/* The channel block contains options pertaining to channels */
  channel {
    /* disable local channels: prevent users from joining &channels.
     * This is extreme, but it is still a flaw in serverhide. It will
     * however remove far more from users than it will give back in
     * security.
     */
    disable_local_channels = no;

    /* invex: Enable/disable channel mode +I, a n!u\@h list of masks
     * that can join a +i channel without an invite.
     */
    use_invex = yes;

    /* except: Enable/disable channel mode +e, a n!u\@h list of masks
     * that can join a channel through a ban (+b).
     */
    use_except = yes;

    /* knock: Allows users to request an invite to a channel that
     * is locked somehow (+ikl).  If the channel is +p or you are banned
     * the knock will not be sent.
     */
    use_knock = yes;

    /* knock delay: The amount of time a user must wait between issuing
     * the knock command.
     */
    knock_delay = 5 minutes;

    /* knock channel delay: How often a knock to any specific channel
     * is permitted, regardless of the user sending the knock.
     */
    knock_delay_channel = 1 minute;

    /* max chans: The maximum number of channels a user can join/be on. */
    max_chans_per_user = 15;

    /* quiet on ban: stop banned people talking in channels. */
    quiet_on_ban = yes;

    /* max bans: maximum number of +b/e/I modes in a channel */
    max_bans = 25;

    /* splitcode: The ircd will now check splitmode every few seconds.
     *
     * Either split users or split servers can activate splitmode, but
     * both conditions must be met for the ircd to deactivate splitmode.
     * 
     * You may force splitmode to be permanent by /quote set splitmode on
     */

    /* split users: when the usercount is lower than this level, consider
     * ourselves split.  this must be set for automatic splitmode
     */
    default_split_user_count = 0;

    /* split servers: when the servercount is lower than this, consider
     * ourselves split.  this must be set for automatic splitmode
     */
    default_split_server_count = 0;

    /* split no create: disallow users creating channels on split. */
    no_create_on_split = no;

    /* split: no join: disallow users joining channels at all on a split */
    no_join_on_split = no;
  };

/* The general block contains many of the options that were once compiled
 * in options in config.h. The general block is read at start time.
 */
  general {
    /* Max time from the nickname change that still causes KILL
     * automatically to switch for the current nick of that user. (seconds)
     */
    kill_chase_time_limit = 90;

    /* If hide_spoof_ips is disabled, opers will be allowed to see the real IP of spoofed
     * users in /trace etc. If this is defined they will be shown a masked IP.
     */
    hide_spoof_ips = yes;

    /* Ignore bogus timestamps from other servers. Yes, this will desync
     * the network, but it will allow chanops to resync with a valid non TS 0
     *
     * This should be enabled network wide, or not at all.
     */
    ignore_bogus_ts = no;

    /* disable auth: completely disable ident lookups; if you enable this,
     * be careful of what you set have_ident to in your auth {} blocks
     */
    disable_auth = no;

    /* disable dns: completely disable DNS reverse lookups; if you enable this,
     * be careful of any hostnames in auth { } blocks.
     */
    disable_dns = no;

    /* disable remote commands: disable users doing commands on remote servers */
    disable_remote_commands = no;

    /* floodcount: the default value of floodcount that is configurable
     * via /quote set floodcount.  This is the amount of lines a user
     * may send to any other user/channel in one second.
     */
    default_floodcount = 10;

    /* anti cgi:irc: Enable this if you have problematic users who evade 
     * server and network bans via CGI:IRC. This is helpful for getting 
     * rid of a user the shadowircd developers know as Vareni. You 
     * should note, however that this may enable users of other clients 
     * unable to connect as well, depending on how closely their gecos
     * matches the standard CGI:IRC gecos...
     *
     * For this reason, this functionality is disabled by default.
     */
    anti_cgi_irc = no; 

    /* failed oper notice: send a notice to all opers on the server when 
     * someone tries to OPER and uses the wrong password, host or ident.
     */
    failed_oper_notice = yes;

    /* dots in ident: the amount of '.' characters permitted in an ident
     * reply before the user is rejected.
     */
    dots_in_ident = 2;

    /* dot in ipv6: ircd-hybrid-6.0 and earlier will disallow hosts 
     * without a '.' in them.  this will add one to the end.  only needed
     * for older servers.
     */
    dot_in_ip6_addr = yes;

    /* min nonwildcard: the minimum non wildcard characters in k/d lines
     * placed via the server.  klines hand placed are exempt from limits.
     * wildcard chars: '.' '*' '?' '@'
     */
    min_nonwildcard = 4;

    /* min nonwildcard simple: the minimum non wildcard characters in 
     * gecos bans.  wildcard chars: '*' '?'
     */
    min_nonwildcard_simple = 3;

    /* max accept: maximum allowed /accept's for +g usermode */
    max_accept = 20;

    /* nick flood: enable the nickflood control code */
    anti_nick_flood = yes;

    /* nick flood: the nick changes allowed in the specified period */
    max_nick_time = 20 seconds;
    max_nick_changes = 5;

    /* anti spam time: the minimum time a user must be connected before
     * custom quit messages are allowed.
     */
    anti_spam_exit_message_time = 5 minutes;

    /* ts delta: the time delta allowed between server clocks before
     * a warning is given, or before the link is dropped.  all servers
     * should run ntpdate/rdate to keep clocks in sync
     */
    ts_warn_delta = 30 seconds;
    ts_max_delta = 5 minutes;

    /* kline reason: show the user the reason why they are netbanned
     * on exit.  may give away who set k/dline when set via tcm.
     */
    kline_with_reason = yes;

    /* kline connection closed: make the users quit message on channels
     * to be \"Connection closed\", instead of the kline reason.
     */
    kline_with_connection_closed = no;

    /* warn no nline: warn opers about servers that try to connect but
     * we don't have a connect {} block for.  Twits with misconfigured 
     * servers can get really annoying with this enabled.
     */
    warn_no_nline = yes;

    /* stats o oper only: make stats o (opers) oper only */
    stats_o_oper_only = yes;

    /* stats P oper only: make stats P (ports) oper only */
    stats_P_oper_only = no;

    /* stats i oper only: make stats i (auth {}) oper only. set to:
     *     yes:    show users no auth blocks, made oper only.
     *     masked: show users first matching auth block
     *     no:     show users all auth blocks.
     */
    stats_i_oper_only = masked;

    /* stats k/K oper only: make stats k/K (klines) oper only.  set to:
     *     yes:    show users no auth blocks, made oper only
     *     masked: show users first matching auth block
     *     no:     show users all auth blocks.
     */
    stats_k_oper_only = masked;

    /* caller id wait: time between notifying a +g user that somebody
     * is messaging them.
     */
    caller_id_wait = 1 minute;

    /* pace wait simple: time between use of less intensive commands
     * (HELP, remote WHOIS, WHOWAS)
     */
    pace_wait_simple = 1 second;

    /* pace wait: time between more intensive commands
     * (ADMIN, INFO, LIST, LUSERS, MOTD, STATS, VERSION)
     */
    pace_wait = 10 seconds;

    /* short motd: send clients a notice telling them to read the motd
     * instead of forcing a motd to clients who may simply ignore it.
     */
    short_motd = no;

    /* ping cookies: require clients to respond exactly to a ping command,
     * can help block certain types of drones and FTP PASV mode spoofing.
     */
    ping_cookie = yes;

    /* no oper flood: increase flood limits for opers. */
    no_oper_flood = yes;

    /* true no oper flood: completely eliminate flood limits for opers
     * and for clients with can_flood = yes in their auth {} blocks
     */
    true_no_oper_flood = yes;

    /* oper pass resv: allow opers to over-ride RESVs on nicks/channels */
    oper_pass_resv = yes;

    /* idletime: the maximum amount of time a user may idle before
     * they are disconnected
     */
    idletime = 0;

    /* maximum links: the maximum amount of servers to connect to for
     * connect blocks without a valid class.
     */
    maximum_links = 1;

    /* REMOVE ME.  The following line checks you've been reading. */
    /* havent_read_conf = 1; */

    /* max targets: the maximum amount of targets in a single 
     * PRIVMSG/NOTICE.  set to 999 NOT 0 for unlimited.
     */
    max_targets = 4;

    /* client flood: maximum amount of data in a clients queue before
     * they are dropped for flooding.
     */
    client_flood = 2560 bytes;

    /* message locale: the default message locale
     * Use \"standard\" for the compiled in defaults.
     * To install the translated messages, go into messages/ in the
     * source directory and run `make install'.
     */
    message_locale = \"standard\";

    /* max silence: the maximum amount of silence entries
     * allowed for a user
     */
    max_silence = 10;

    /* servlink path: path to 'servlink' program used by ircd to handle
     * encrypted/compressed server <-> server links.
     *
     * only define if servlink is not in same directory as ircd itself.
     */
    /* servlink_path = \"/usr/local/ircd/bin/servlink\"; */

    /* default cipher: default cipher to use for cryptlink when none is
     * specified in connect block.
     */
    /* default_cipher_preference = \"BF/168\"; */

    /* use egd: if your system does not have *random devices yet you
     * want to use OpenSSL and encrypted links, enable this.  Beware -
     * EGD is *very* CPU intensive when gathering data for its pool
     */
    /* use_egd = yes; */

    /* egdpool path: path to EGD pool. Not necessary for OpenSSL >= 0.9.7
     * which automatically finds the path.
     */
    /* egdpool_path = \"/var/run/egd-pool\"; */


    /* compression level: level of compression for compressed links between
     * servers.  
     *
     * values are between: 1 (least compression, fastest)
     *                and: 9 (most compression, slowest).
     */
    /* compression_level = 6; */

    /* throttle time: the minimum amount of time between connections from
     * the same ip.  exempt {} blocks are excluded from this throttling.
     * Offers protection against flooders who reconnect quickly.  
     * Set to 0 to disable.
     */
    throttle_time = 10;
  };

  /* wingate: the wingate block defines options concerning the wingate
   *          message sent to users.
   */
  wingate {
    /* wingate enable: Enable wingate scanning notices. */
    wingate_enable = yes;

    /* monitorbot: Sets the address of the monitorbot. */
    monitorbot = \"127.0.0.1\";

    /* wingate webpage: URL for more information. */
    wingate_webpage = \"http://kline.synatek.net/proxy/\";
  };
  
    hiding {
    /* whois hiding: displays a fake server for whois */");
    &b("whois_hiding","Fake server for whois","*.moonet.com");

&a("    /* whois description: description for the fake server. */");
    &b("whois_description","Description for the fake server","MooNet IRC network");

&a("    /* roundrobin: roundrobin server used for links flattening. */");
    &b("roundrobin","Roundrobin server used for links flattening","irc.moonet.com");
&a("  };

  modules {
    /* module path: other paths to search for modules specified below
     * and in /modload.
     */");
    print "IRCd Prefix: [/usr/local/ircd] "; $rv = <STDIN>; chomp $rv; $rv = "/usr/local/ircd" if (!$rv);
&a("    path = \"$rv\/modules\";
    path = \"$rv\/modules/autoload\";

    /* module: the name of a module to load on startup/rehash */
    /* module = \"some_module.so\"; */
  };
  
  /*** OPERATOR BLOCKS *************************************************/
  operator {
    /* name: the name of the oper */
    name = \"god\";

    /* user: the user\@host required for this operator.  CIDR is not
     * supported.  multiple user=\"\" lines are supported.
     */
    user = \"*god@*\";
    user = \"*@127.0.0.1\";

    /* password: the password required to oper. (use mkpasswd to encrypt) */
    password = \"etcnjl8juSU1E\";

    /* encrypted: whether or not the password above is encrypted. */
    encrypted = yes;

    /* rsa key: the public key for this oper when using Challenge.
     * A password should not be defined when this is used, see 
     * doc/challenge.txt for more information.
     */
    /* rsa_public_key_file = \"/usr/local/ircd/etc/oper.pub\"; */

    /* class: the class the oper joins when they successfully /oper */
    class = \"opers\";

    /* flags { } subblock: designates the permissions the operator
     * has. Note that outside the flags { } subblock, the old permissions
     * sets work too.
     *
     * For more information on what each permission does, see the rendered
     * documentation from the Shadow Documentation Project, included with
     * the ircd in doc/sdp of the ircd source tree.
     */

    flags {
      global_kill;
      remote;
      kline;
      unkline;
      xline;
      die;
      rehash;
      nick_changes;
      admin;
      /* hidden_admin; */
      auspex;
      set_owncloak;
      set_anycloak;
      immune;
      override;
      grant;
      netadmin;
      techadmin;
      routing;
      wants_whois;
    };

  };
  
  
  /*** LINK BLOCKS ********************************************************/
  connect {
    /* name: the name of the server */
    name = \"irc.uplink.com\";

    /* host: the host or IP to connect to.  If a hostname is used it
     * must match the reverse dns of the server.
     */
    host = \"192.168.0.1\";

    /* passwords: the passwords we send (OLD C:) and accept (OLD N:).
     * The remote server will have these passwords reversed.
     */
    send_password = \"password\";
    accept_password = \"anotherpassword\";

    /* encrypted: controls whether the accept_password above has been
     * encrypted.  (OLD CRYPT_LINK_PASSWORD now optional per connect)
     */
    encrypted = no;

    /* port: the port to connect to this server on */
    port = 6666;

    /* hub mask: the mask of servers that this server may hub. Multiple
     * entries are permitted
     */
    hub_mask = \"*\";

    /* leaf mask: the mask of servers this server may not hub.  Multiple
     * entries are permitted.  Useful for forbidding EU -> US -> EU routes.
     */
    /* leaf_mask = \"*.uk\"; */

    /* class: the class this server is in */
    class = \"server\";

    flags {
      autoconn;
      compressed;
      /* lazylink; */
    };

    /* masking: the servername we pretend to be when we connect */
    /* fakename = \"*.arpa\"; */
  };

  connect {
    name = \"ipv6.some.server\";
    host = \"3ffd:dead:beef::1\";
    send_password = \"password\";
    accept_password = \"password\";
    port = 6666;

    /* aftype: controls whether the connection uses \"ipv4\" or \"ipv6\".
     * Default is ipv4.
     */
    aftype = ipv6;
    class = \"server\";
  };
};");

system("mv ircd.conf ircd.conf.old") if (-e "ircd.conf");
open(IRCDCONF, ">>ircd.conf");
print IRCDCONF "$cfile";
close IRCDCONF;

print "\n\n----------------------------------------------\n\nNow edit the oper and connect blocks in ircd.conf and copy ircd.conf to the etc/ directory of your ircd server, and enjoy!\n\n\n";
