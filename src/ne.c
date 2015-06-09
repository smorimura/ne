/*--------------------------------------------------------------------
	nxeditor
			FILE NAME:nxedit.c
			Programed by : I.Neva
			R & D  ADVANCED SYSTEMS. IMAGING PRODUCTS.
			1992.06.01

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 
--------------------------------------------------------------------*/
#define	VAL_impl

#include "ed.h"
#include "sh.h"
#include <ctype.h>

#include	<errno.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<pwd.h>
#include	<fcntl.h>

#if HAVE_SYS_WAIT_H
#	include <sys/wait.h>
#endif

#ifdef	HAVE_SYS_UTSNAME_H
#	include	<sys/utsname.h>
#endif


	char *title1 = "        Multi FILE Editor      ";
	char *title2 = "               Ne              ";
	char *copyright = "Copyright (c) 1991, 1992 I.Nebashi";

static	void	opening()
{
/* Emacs 的ガイド表示をいれる。 */

//	term_cls();
//	term_color_normal();

//	term_flush();

//	term_puts("ne " VER);
//	term_inkey();
}


/*-------------------------------------------------------------------
	Main Command Loop
-------------------------------------------------------------------*/

#ifdef	__dead2
void	ne_loop(int region) __dead2;
#endif

void	ne_loop(int region)
{
 	int 	key;

	for (;;) 
		{
		 if (region==0)
		 	CrtDrawAll();

		 dsp_allview();
		 term_csrn();
		 key=get_keyf(region);
		 RefreshMessage();

		 if (region==0&& csrse.gf)
		 	{
		 	 csrse.gf=FALSE;
		 	 term_locate(GetRow(), GetCol()+NumWidth);
		 	 crt_crmark();
		 	}

		 if (key==-1)
		 	continue;

		 if ((key&KF_normalcode)==0)
		 	funclist[0][key](); else
		 	{
		 	 if (region==0)
		 	 	InputAndCrt(key& ~KF_normalcode); 
		 	}
		}
	/*NOTREACHED*/
}

void	ne_fin()
{
	exit(0);
	/*NOTREACHED*/
}


void	sysinfo_path(char *s,const char *t)
{
	sprintf(s, "%s/%s", sysinfo.nxpath, t);
}

void	ne_init()
{
	char	*p;

	sysinfo.vp_def=hash_init(NULL, MAX_val);
	opt_default();

	p=getenv("HOME");
	if (p!=NULL)
		strcpy(sysinfo.nxpath, p); else
		{
		 struct passwd	*pw;
		 pw=getpwuid(geteuid());
		 if (pw!=NULL)
		 	strcpy(sysinfo.nxpath, pw->pw_dir); else
			getcwd(sysinfo.nxpath, LN_path);
		}
	strcat(sysinfo.nxpath, "/.ne");
	mkdir(sysinfo.nxpath, 0777); // !!

	sysinfo.shell=getenv("SHELL");
	if (sysinfo.shell==NULL)
		sysinfo.shell="/bin/sh";

	config_read("nerc");
}

void	ne_report()
{
#ifndef	HAVE_UNAME
	report_puts("ne version " VER "\n");

#else
	struct utsname	u;

	uname(&u);
	report_printf("ne version " VER "  %s %s (%s)\n"
		, u.sysname, u.release, u.version);

#endif

	report_puts("***\n");
	report_puts("環境変数:\n");
	report_printf("  $HOME: %s\n", getenv("HOME"));
	report_printf("  $TERM: %s\n", getenv("TERM"));
	report_printf("  $NE_TERM: %s\n", getenv("NE_TERM"));
	report_printf("  $SHELL: %s\n", getenv("SHELL"));
	report_puts("\n");

	hash_report(sysinfo.vp_def);
	report_puts("\n");

	term_report();
	report_puts("\n");

	keydef_report();
	report_puts("***\n");

}

bool	ne_arg(int argc,char *argv[])
{
	int 	line;
	int 	optcount;
	bool	pf;
	char	buf[LN_dspbuf+1];
	bool	f;
	int 	c;
	char	*sp,*p;

	line=0;
	f=FALSE;
	pf=FALSE;
	sysinfo.f_report=FALSE;

	for (optcount=1; optcount<argc; ++optcount) // ????
		{
		 c=getopt(argc, argv, "Vrjeod:D:");
		 if (c==EOF)
		 	break;

		 switch(c) 
		 	{
		  case 'V':
		  	 sysinfo.f_report=TRUE;
		  	 report_set(stderr);
		  	 break;
		  case 'j':
		  	 hash_set(sysinfo.vp_def, "japanese","true");
		  	 break;
		  case 'e':
		  	 hash_set(sysinfo.vp_def, "japanese","false");
		  	 break;
		  case 'r':
		  	 pf=TRUE;
		  	 break;
		  case 'o':
		  	 opening();
		  	 break;
		  case 'D':
		  	 strcpy(buf,optarg);
		  	 optcount++;
		  	 sp=buf;
		  	 p=strsep(&sp,"=");
		  	 hash_set(sysinfo.vp_def, buf, p);
		 	}
		}

	sysinfo_optset();

	if (sysinfo.f_report)
		return FALSE;

	if (pf)
		return profile_read();

	for (; optcount<argc; ++optcount)
		{
		 if (*argv[optcount]=='+')
		 	line=atoi(argv[optcount]+1); else
		 	{
		 	 if (FileOpenOp(argv[optcount]))
		 	 	f=TRUE;
		 	}
		}

	if (!f&& line>0)
		{
		 int	a;

		 a=min(line,GetRowWidth()/2+1);

		 csr_setly(line-a+1);
		 csr_setdy(a);
		}

	if (!f)
		FileOpenOp("untitled.txt");

	return f;
}

int 	main(int argc,char *argv[])
{
	term_init();
	term_start();
	term_cls();
	dsp_allinit();
	SetSignal();

	lists_init();
	edbuf_init();

	keydef_init();
	ne_init();
	ne_arg(argc, argv);
	key_set();
	sysinfo_optset();

	if (sysinfo.f_report)
		{
		 ne_report();
		 exit(0);
		}

	udbuf_init();
	search_init();
	eff_init(NULL,NULL);
	system_guide_init();
	*sysinfo.doublekey='\0'; // !!

	ne_loop(0);
	/* NOTREACHED */
}



/*-------------------------------------------------------------------
	Escape Shell
-------------------------------------------------------------------*/
void	CommandCom(char *sys_buff)
{
	bool	f;

	term_stop();

	if (*sys_buff == '\0')
		{
		 puts(TYPE_EXIT_MSG);
		 system(sysinfo.shell);
		 f=FALSE;
		} else 
		{
		 puts(sys_buff);
		 system(sys_buff);
		 f=TRUE;
		}

	term_start();

	if (f)
		{
		 fputs(HIT_ANY_KEY_MSG, stdout);
		 fflush(stdout);
		 term_kflush();
		 term_inkey();
		}

	term_cls();
}

SHELL	void	op_misc_exec()
{
	char	buf[MAXEDITLINE+1];

	*buf='\0';
	if (HisGets(buf, GETS_SHELL_MSG, SHELLS_SYSTEM) != NULL)
		CommandCom(buf);
}


SHELL	void	op_opt_linenum()
{
	opt_set("number", NULL);

//	OffsetSetByColumn();
}

SHELL	void	op_misc_redraw()
{
	term_cls();
}



/*-----------------------------------------------------------------------------
	fork shell and take its stdout/stderr.
*/
SHELL	void	op_misc_insert_output( void )
{
	pid_t	pid_child ;
	int		pipefds[2] ;
	char	buf[MAXEDITLINE+1]="";


	if (HisGets(buf, GETS_SHELL_MSG, SHELLS_SYSTEM) == NULL)
		return ;
		/* NOTREACHED */

	/*	Normally, this may not be failed, but... */
	if (pipe(pipefds)!=0)
		{
		 system_msg(strerror(errno));
		 term_inkey();
		 return;
		/* NOTREACHED */
		}

	pid_child=fork();
	switch(pid_child)
		{
		/*	now we are child. */
		case	0 :
			/*	pipe plumbing.
				child process's stdout and stderr are dup'ed from same fd.
				whatever written into these will end up to our pipefds[ 0 ].
			*/
//			term_stop();

			close( 1 ) ;
			dup( pipefds[ 1 ] ) ;	/* duplicate to stdout */

			close( 2 ) ; 
			dup( pipefds[ 1 ] ) ;	/* duplicate to stderr */

			/*	these fds are no longer useful. */
			close( pipefds[ 0 ] ) ;
			close( pipefds[ 1 ] ) ;

			execl(sysinfo.shell, sysinfo.shell, "-c", buf, NULL) ;	/* should not be failed. */
			_exit( 1 ) ;
			/* NOTREACHED */

		/*	cannot be forked off. */
		case	-1 :
			system_msg(strerror(errno));
			term_inkey();
			system_msg("");

			close(pipefds[0]);
			close(pipefds[1]);

			break;
			/* NOTREACHED */

	/*	now we are parent. */
		default :
			{
			int	status ;
			FILE	*fp_pipe;

			close(pipefds[1]);
			fp_pipe = fdopen(pipefds[ 0 ], "r");
			if (fp_pipe==NULL)
				{
				 system_msg(strerror(errno));
				 term_inkey();
				 system_msg("");
				} else
				{
				 EditLine	*ed, *ed_new;
				 bool	f;
				 int 	n;

				 csr_leupdate();
				 ed_new=ed=GetList(GetLineOffset()-1);
				 f=FALSE;

				 term_stop();

				 while ( fgets(buf, sizeof(buf) - 1, fp_pipe) )
				 	{
				 	/* echo to stdout for user friendliness. :) */
				 	fputs(buf, stdout);

				 	/* remove last '\n' */
				 	n=strlen(buf)-1;
				 	if (buf[n] == '\n')
				 		buf[n] = '\0' ;

				 	/*	create new linebuffer and insert into list. */
				 	InsertLine(ed, MakeLine(buf));
				 	ed=ed->next;
				 	f=TRUE;
				 	}
				 fclose( fp_pipe ) ;
				 close(pipefds[0]);
				 term_start();

				 if (f)
				 	{
				 	 SetFileChangeFlag( ) ;	/* now, file is dirty. */
				 	 csrse.ed=ed_new->next;
				 	 csr_lenew();
				 	 OffsetSetByColumn();
				 	}
				}
			wait( &status ) ;

			term_cls();

			}
		}
}

