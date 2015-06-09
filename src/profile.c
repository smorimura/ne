/*--------------------------------------------------------------------
  safty profile I/O module.

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 
--------------------------------------------------------------------*/
#include "ed.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>


static	bool	safty_write(int fd)
{
	FILE	*fp;
	EditLine	*ed;

	fp=fdopen(fd, "w"); 
	if (fp==NULL) 
		return FALSE;

	ed=GetTop()->next;
	while (ed!=NULL && ed->buffer!=NULL)
		{
		 if (ed->next==NULL)
		 	{
		 	 if (*ed->buffer!='\0')
		 	 	fputs(ed->buffer, fp);
		 	 break;
		 	}
		 fprintf(fp, "%s\n", ed->buffer);
	 	 ed = ed->next;
		}

	fclose(fp);

	return TRUE;
}

void	profile_write()
{
	int 	i;
	char	buf[LN_path+1], savedir[LN_path+1];
	FILE	*fp;
	struct	stat	st;

	char	sp[MAX_edfiles][LN_path+1];
	bool	mf;
	int 	n;

	mf=FALSE;
	for (i=0; i<MAX_edfiles; i++)
		{
		 strcpy(sp[i], edbuf[i].path);
		 if (*edbuf[i].path!='\0' && edbuf[i].cf)
		 	mf=TRUE;
		}

	if (mf)
		{
		 sysinfo_path(savedir, ".neTmp");	// !!
		 if (stat(savedir, &st) < 0)
		 	{
		 	 if (mkdir(savedir, 0700)<0)
		 	 	{
		 	 	 fprintf(stderr, "Unable to create directory \"%s\"\n", savedir);
		 	 	 perror("");
		 	 	 return;
		 	 	}
		 	}

		 for (i=0; i<MAX_edfiles; i++)
		 	{
		 	 if (*sp[i]=='\0' || !edbuf[i].cf)
		 	 	continue;

		 	 CurrentFileNo=i;
		 	 for (n=0;;++n)
		 	 	{
		 	 	 int	fd;
		 	 	 if (n>=10) // retry
		 	 	 	{
		 	 	 	 perror("");
		 	 	 	 break;
		 	 	 	}
		 	 	 sprintf(buf, "%s/XXXXXX",savedir);
		 	 	 fd=mkstemp(buf);
		 	 	 if (fd==-1 || !safty_write(fd))
		 	 	 	continue;

		 	 	 strcpy(sp[i], buf);
		 	 	 chmod(buf, 0600);
		 	 	 break;
		 	 	}
		 	}
		}

	sysinfo_path(buf, ".neprof");
	fp=fopen(buf,"w");
	if (fp==NULL)
		{
		 fprintf(stderr, "Unable to create .neprof \"%s\"\n", buf);
		 perror("");
		 return;
		}

	fprintf(fp,"%s\n", PROFILE_VERSION);
	for (i=0; i<MAX_edfiles; i++)
		{
		 if (*sp[i]!='\0')
		 	{
		 	 fprintf(fp,"%s:%ld:\t%s:%d:%d\n" ,sp[i] ,edbuf[i].se.ly
		 	 	,edbuf[i].path ,edbuf[i].kc ,edbuf[i].rm);
		 	}
		}
	fclose(fp);
}

int 	profile_read()
{
	FILE	*fp;
	char	buf[LN_path+1],fn[LN_path+1];
	char	tmp[MAXEDITLINE+1];
	char	*p,*sp,*q;
	int 	i;
	long	jl;
	struct stat	sbuf;

	sysinfo_path(fn, ".neprof");
	fp=fopen(fn,"r");
	if (fp==NULL) 
		{
failed:
/*fprintf(stderr,"failed\n");*/
		 return FALSE;
		}
	fgets(buf, LN_path, fp);
	i=strlen(buf);
	if (i>0 && buf[i-1]=='\n')
		buf[i-1] = '\0';
	if (strcmp(buf,PROFILE_VERSION)!=0)
		goto failed;

	system_msg(WAITING_MSG);
	while (fgets(buf,LN_path,fp)!=NULL) 
		{
		 sp=buf;
		 q=p=strsep(&sp,":");
		 if (!FileOpenOp(p))
		 	continue;

		 p=strsep(&sp,":");
		 if (p==NULL)
		 	continue;
		 jl=atol(p);
		 csr_setly(jl);

		 p=strsep(&sp,":");
		 if (p==NULL)
		 	continue;
		 while(isspace(*p))
		 	++p;
		 if (strcmp(edbuf[CurrentFileNo].path, p)!=0)
		 	{
			 if (edbuf[CurrentFileNo].ct==-1&&GetLastNumber()==1)
			 	{
			 	 sprintf(tmp,"profile: ファイル`%s'が失われました。",p);
				 inkey_wait(tmp);

			 	 edbuf_rm(CurrentFileNo);
			 	 lists_clear();

			 	 FileOpenOp(p);
			 	 csr_setly(jl);
			 	} else
			 	{
				 //!! RenameFile(CurrentFileNo, p);
				 strcpy(fn,p);
				 reg_pf(NULL, fn, FALSE);

			 	 strcpy(edbuf[CurrentFileNo].path,fn);
		 	 	 SetFileChangeFlag();
			 	 edbuf[CurrentFileNo].ct= stat(p,&sbuf)==-1?
			 	 	-1: sbuf.st_ctime;

			 	 edbuf_rm(CurrentFileNo);
			 	 edbuf_add(fn);

			 	 unlink(q);
			 	}
			}

		 p=strsep(&sp,":");
		 if (p==NULL)
		 	continue;
		 edbuf[CurrentFileNo].kc= atoi(p)%3;

		 p=strsep(&sp,":");
		 if (p==NULL)
		 	continue;
		 edbuf[CurrentFileNo].rm= atoi(p)%3;
		}
	fclose(fp);

	sysinfo_path(fn, ".neprof");
	unlink(fn);
	sysinfo_path(fn, ".neTmp");
	rmdir(fn);

	return TRUE;
}

