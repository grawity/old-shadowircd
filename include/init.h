/*
 * init.h
 * HybServ2 Services by HybServ2 team
 *
 * $Id: init.h,v 1.1 2003/12/16 19:52:38 nenolod Exp $
 */

#ifndef INCLUDED_init_h
#define INCLUDED_init_h

struct Luser;

/*
 * Prototypes
 */

void ProcessSignal(int signal);
void InitListenPorts();
void InitLists();
void InitSignals();
void PostCleanup();
void InitServs(struct Luser *servptr);

extern int control_pipe;

#endif /* INCLUDED_init_h */
