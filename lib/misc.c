/*
 *    misc module.
 *
 * Copyright (c) 1999, 2000 SASAKI Shunsuke.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification, immediately at the beginning of the file.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * Where this Software is combined with software released under the terms of 
 * the GNU Public License ("GPL") and the terms of the GPL would require the 
 * combined work to also be released under the terms of the GPL, the terms
 * and conditions of this License will apply in addition to those of the
 * GPL with the exception of any terms or conditions of this License that
 * conflict with, or are expressly prohibited by, the GPL.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include	"config.h"

#include	<stdio.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<utime.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	"generic.h"

#define	iseuc(c)	( (u_char)(c)>=0xa1 &&(u_char)(c)<=0xfe )
//	|| c==0x8e;


  /* ☆ misc */

void	strjncpy(char *s,const char *t,size_t ln)
{
	int 	i;
	bool	kf;
	char	*p;

	p=s;
	if (strlen(t)<=ln)
		{
		 strcpy(p,t);
		 return;
		}

	kf=FALSE;
	for (i=0;i<ln-1;++i,++p,++t)
	{
		*p=*t;
		if (kf)
			kf=FALSE; else
			kf=iseuc(*p);
	}

	if (kf|| !iseuc(*t))
		*p++=*t;
	*p='\0';
}

int 	touchfile(const char *path, time_t atime, time_t mtime)
{
	struct utimbuf times;

	times.actime = atime;
	times.modtime = mtime;
	return utime(path, &times);

/*
	struct timeval tvp[2];

	tvp[0].tv_sec = atime;
	tvp[0].tv_usec = 0;
	tvp[1].tv_sec = mtime;
	tvp[1].tv_usec = 0;
	return utimes(path, tvp);
*/
}

bool	mole_dir(const char* s)
{
	const	char	*p;
	char	buf[1024+1];	// !! buf size
	struct	stat	st;

	p=s;
	if (*p=='/')
		++p;

	for(;;)
		{
		 if (*p=='\0'||p==NULL)
		 	return TRUE;
		 p=strchr(p,'/');
		 if (p==NULL)
		 	strcpy(buf,s); else
		 	{
		 	 memcpy(buf,s,p-s);
		 	 buf[p-s]='\0';
		 	 ++p;
		 	}
//fprintf(stderr,"mole_chk[%s]\n",buf);
		 if (stat(buf,&st)==0)
		 	{
		 	 if ((st.st_mode&S_IFMT)==S_IFDIR)
		 	 	continue;
		 	 return FALSE;
		 	}
		 if (mkdir(buf,0777)<0)
		 	return FALSE;
		}
	return TRUE;
}


bool	kanji_chk(const char *s,const char *t)
{
	int 	r=0;

//fprintf(stderr, "%2d[%s]", (int)t-(int)s, s);

	--t;
	while(iseuc(*t) && s<=t)
		{
		 ++r;
		 --t;
		}

//fprintf(stderr, "%d\n", r);
	return (r%2)==1;
}

/*

  strstr, mkdir などがない環境はここにそのコードを追加

*/

#ifndef	HAVE_FLOCK

#	ifndef	F_WRLCK
int 	flock(int fd, int op)
{
	return 0;
}

#else

#ifndef	LOCK_SH
#define	LOCK_SH	1
#define	LOCK_EX	2
#define	LOCK_NB	4
#define	LOCK_UN	8
#endif

int 	flock(int fd, int op)
{
	struct	flock	fl;
	int 	cmd;

	fl.l_start=0;
	fl.l_len=0;
	fl.l_whence=SEEK_SET;

	if (op&LOCK_EX)
		fl.l_type=F_WRLCK;
	if (op&LOCK_SH)
		fl.l_type=F_RDLCK;
	if (op&LOCK_UN)
		fl.l_type=F_UNLCK;

	cmd= (op&LOCK_NB)? F_SETLK : F_SETLKW;
	return fcntl(fd, cmd, &fl);
}

#	endif

#endif


#ifndef	HAVE_STRSEP
char	*strsep(char **stringp, const char *delim)
{
	char	*p;

	p=*stringp;

	if (p==NULL)
		return p;

	while (**stringp !='\0')
		{
		 if (strchr(delim, **stringp)!=NULL)
		 	{
		 	 **stringp = '\0';
		 	 ++*stringp;
		 	 return p;
		 	}
		 ++*stringp;
		}
	*stringp=NULL;
	return p;
}

#endif


#ifndef	HAVE_REALPATH
void	realpath(const char *cp, char *s)
{
	strcpy(s, cp);
	reg_pf(NULL, s, TRUE);
}
#endif

