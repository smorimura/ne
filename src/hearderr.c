/*--------------------------------------------------------------------
	nxeditor
			FILE NAME:hearderr.c
			Programed by : I.Neva
			R & D  ADVANCED SYSTEMS. IMAGING PRODUCTS.
			1992.06.01

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 
--------------------------------------------------------------------*/
#include	"ed.h"
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<signal.h>

bool	failed=FALSE;

#if !defined(SIGIOT) && defined(SIGABRT)
#define	SIGIOT	SIGABRT
#endif

int 	HearderrEnd(char *message)
{
	static	char	buf[LN_path+1];

	if (failed)
		exit(2);
	failed=TRUE;

	profile_write();

	sysinfo_path(buf, "ne_lock");
	unlink(buf);

//	fprintf(stderr,"%s\n",message);
	if (message==NULL)
		perror(""); else
	fprintf(stderr,"%s\n",message);

	exit(1);
}


RETSIGTYPE	signalALRM()	{profile_write();}
RETSIGTYPE	signalHUP()	{HearderrEnd("signal.. (SIGHUP) hangup");}
RETSIGTYPE	signalILL()	{HearderrEnd("sagnal.. (SIGILL)");}
RETSIGTYPE	signalPIPE(){HearderrEnd("signal.. (SIGPIPE)");}
RETSIGTYPE	signalTERM(){HearderrEnd("signal.. (SIGTERM) software termination signal");}
RETSIGTYPE	signalBUS()	{HearderrEnd("signal.. (SIGBUS) bus error");}
RETSIGTYPE	signalSEGV(){HearderrEnd("signal.. (SIGSEGV) segmentation violation");}
RETSIGTYPE	signalIOT()	{HearderrEnd("signal.. (SIGIOT)");}
RETSIGTYPE	signalEMT()	{HearderrEnd("signal.. (SIGEMT)");}
RETSIGTYPE	signalSYS()	{HearderrEnd("signal.. (SIGSYS)");}
RETSIGTYPE	signalURG()	{HearderrEnd("signal.. (SIGURG) urgent condition present on soket");}
RETSIGTYPE	signalXCPU(){HearderrEnd("signal.. (SIGXCPU) cpu time limit exceeded (see getrlimit(2))");}


void	SetSignal()
{
	signal(SIGALRM, signalALRM);

	signal(SIGHUP, signalHUP);
	signal(SIGILL, signalILL);
#ifdef	SIGIOT
	signal(SIGIOT, signalIOT);
#endif
#ifdef	SIGEMT
	signal(SIGEMT, signalEMT);
#endif
	signal(SIGBUS, signalBUS);
	signal(SIGSEGV,signalSEGV);
#ifdef	SIGSYS
	signal(SIGSYS, signalSYS);
#endif

	signal(SIGPIPE,signalPIPE);
	signal(SIGTERM,signalTERM);
#ifdef	SIGURG
	signal(SIGURG, signalURG);
#endif
#ifdef	SIGXCPU
	signal(SIGXCPU,signalXCPU);
#endif
}
