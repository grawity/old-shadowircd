/*
 * NetworkBOPM: The ShadowIRCd Anti-Proxy Solution
 * conf.c: Configuration Parser
 *
 * $Id: conf.c,v 1.1 2004/05/24 23:22:41 nenolod Exp $
 */

#include "netbopm.h"

#define PARAM_ERROR(ce) { printf("%s:%i: no parameter for " \
                          "configuration option: %s\n", \
                          (ce)->ce_fileptr->cf_filename, \
                          (ce)->ce_varlinenum, (ce)->ce_varname); \
  return 1; }

static int c_serverinfo(CONFIGENTRY *);

static int c_si_name(CONFIGENTRY *);
static int c_si_desc(CONFIGENTRY *);
static int c_si_uplink(CONFIGENTRY *);
static int c_si_port(CONFIGENTRY *);
static int c_si_pass(CONFIGENTRY *);
static int c_si_sid(CONFIGENTRY *);

struct ConfTable
{
  char *name;
  int rehashable;
  int (*handler) (CONFIGENTRY *);
};

/* *INDENT-OFF* */

static struct ConfTable conf_root_table[] = {
  { "SERVERINFO", 1, c_serverinfo },
  { NULL, 0, NULL }
};

static struct ConfTable conf_si_table[] = {
  { "NAME",        0, c_si_name        },
  { "DESCRIPTION", 0, c_si_desc        },
  { "UPLINK",      0, c_si_uplink      },
  { "PORT",        0, c_si_port        },
  { "PASSWORD",    1, c_si_pass        },
  { "SID",         0, c_si_sid         },
  { NULL, 0, NULL }
};

/* *INDENT-ON* */

void conf_parse(void)
{
  CONFIGFILE *cfptr, *cfp;
  CONFIGENTRY *ce;
  struct ConfTable *ct = NULL;

  cfptr = cfp = config_load(config_file);

  if (cfp == NULL)
  {
    printf("conf_parse(): unable to open configuration file: %s\n",
         strerror(errno));

    exit(1);
  }

  for (; cfptr; cfptr = cfptr->cf_next)
  {
    for (ce = cfptr->cf_entries; ce; ce = ce->ce_next)
    {
      for (ct = conf_root_table; ct->name; ct++)
      {
        if (!strcasecmp(ct->name, ce->ce_varname))
        {
          ct->handler(ce);
          break;
        }
      }
      if (ct->name == NULL)
      {
        printf("%s:%d: invalid configuration option: %s\n",
             ce->ce_fileptr->cf_filename, ce->ce_varlinenum, ce->ce_varname);
      }
    }
  }

  config_free(cfp);
}

void conf_init(void)
{
  if (me.password)
    free(me.password);
  if (me.uplink)
    free(me.uplink);
  if (me.gecos)
    free(me.gecos);
  if (me.sid)
    free(me.sid);
  if (me.svname)
    free(me.svname);  

  me.password = me.uplink = me.gecos = me.sid = me.svname = NULL;
  me.port = 0;
}

static int subblock_handler(CONFIGENTRY *ce, struct ConfTable *table)
{
  struct ConfTable *ct = NULL;

  for (ce = ce->ce_entries; ce; ce = ce->ce_next)
  {
    for (ct = table; ct->name; ct++)
    {
      if (!strcasecmp(ct->name, ce->ce_varname))
      {
        ct->handler(ce);
        break;
      }
    }
    if (ct->name == NULL)
    {
      printf("%s:%d: invalid configuration option: %s\n",
           ce->ce_fileptr->cf_filename, ce->ce_varlinenum, ce->ce_varname);
    }
  }
  return 0;
}

static int c_serverinfo(CONFIGENTRY *ce)
{
  subblock_handler(ce, conf_si_table);
  return 0;
}

static int c_si_name(CONFIGENTRY *ce)
{
  if (ce->ce_vardata == NULL)
    PARAM_ERROR(ce);

  me.svname = sstrdup(ce->ce_vardata);

  return 0;
}

static int c_si_desc(CONFIGENTRY *ce)
{
  if (ce->ce_vardata == NULL)
    PARAM_ERROR(ce);

  me.gecos = sstrdup(ce->ce_vardata);

  return 0;
}

static int c_si_uplink(CONFIGENTRY *ce)
{
  if (ce->ce_vardata == NULL)
    PARAM_ERROR(ce);

  me.uplink = sstrdup(ce->ce_vardata);

  return 0;
}

static int c_si_port(CONFIGENTRY *ce)
{
  if (ce->ce_vardata == NULL)
    PARAM_ERROR(ce);

  me.port = ce->ce_vardatanum;

  return 0;
}

static int c_si_pass(CONFIGENTRY *ce)
{
  if (ce->ce_vardata == NULL)
    PARAM_ERROR(ce);

  me.password = sstrdup(ce->ce_vardata);

  return 0;
}

static int c_si_sid(CONFIGENTRY *ce)
{
  if (ce->ce_vardata == NULL)
    PARAM_ERROR(ce);

  me.sid = sstrdup(ce->ce_vardata);

  return 0;
}

static void copy_me(me_t *src, me_t *dst)
{
  dst->password = sstrdup(src->password);
  dst->svname = sstrdup(src->svname);
  dst->port = src->port;
  dst->gecos = sstrdup(src->gecos);
  dst->uplink = sstrdup(src->uplink);  
  dst->sid = sstrdup(src->sid);
}

int conf_rehash(void)
{
  me_t *hold_me = scalloc(sizeof(me_t), 1);   /* and keep_me_warm; */
  copy_me(&me, hold_me);

  /* reset everything */
  conf_init();

  /* now reparse */
  conf_parse();

  /* now recheck */
  if (!conf_check())
  {
    printf("conf_rehash(): conf file was malformed, aborting rehash\n");

    /* return everything to the way it was before */
    copy_me(hold_me, &me);

    free(hold_me);

    return 0;
  }

  return 1;
}

int conf_check(void)
{
  if (!me.svname)
  {
    printf("conf_check(): no `name' set in %s\n", config_file);
    return 0;
  }

  if (!me.gecos)
    me.gecos = sstrdup("NetworkBOPM");

  if (!me.uplink)
  {
    printf("conf_check(): no `uplink' set in %s\n", config_file);
    return 0;
  }

  if (!me.port)
  {
    printf("conf_check(): no `port' set in %s; defaulting to 6667\n",
         config_file);
    me.port = 6667;
  }

  if (!me.password)
  {
    printf("conf_check(): no `password' set in %s\n", config_file);
    return 0;
  }

  if (!me.sid)
  {
    printf("conf_check(): no `sid' set in %s\n", config_file);
    return 0;
  }

  return 1;
}
