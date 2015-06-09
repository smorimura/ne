/*--------------------------------------------------------------------
  filer 0.2.7

    Copyright (c) 1997,1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 
--------------------------------------------------------------------*/
#include	"filer.h"
#include	"sh.h"
#include	<ctype.h>

#if	TIME_WITH_SYS_TIME
#	include	<sys/time.h>
#	include	<time.h>
#else
#	if HAVE_SYS_TIME_H
#		include	<sys/time.h>
#	else
#		include	<time.h>
#	endif
#endif

extern	int 	touchfile(const char *path, time_t atime, time_t mtime);



#if 0
typedef	struct	fitem
{
	char			*fn;
	char			*e;
	struct	stat	stat;

	struct	fitem	*next;
}	fitem_t;

typedef	struct
{
	fitem_t	*fitem;
	int 	n;
}	flist_t;

typedef	struct
{
	char	path[LN_path +1];

	menu_t	menu;
	bool	df;

	flist_t	flist;
	fitem_t	**findex;
	int 	*mark;
	int 	markmax;
	int 	marknum;

	char	match[LN_path+1];
}	fw_t;
fw_t	fw[2];
#define	fw_c	fw[eff.wa]

typedef	struct
{
	int 	wn;			/* windowの数 */
	int 	wa;			/* Window Active 位置 */
	bool	df;			/* dirty flag */

	int 	sort[2];	/* sort方式 */
}	eff_t;
eff_t	eff;

enum	{SA_none,SA_fname,SA_ext};

typedef struct	fop
{
	int 	(*file_func)(const char*,struct stat *,const char*, struct fop*);
	int 	(*dir_func)(const char*,struct stat *,const char*
														,bool, struct fop*);
	char    *title;

	int 	om;		/* overwrite mode. */
	int 	pfm;	/* permition force mode. */

	int 	en;		/* error number */
}	fop_t;

#endif
















/* ディレクトリ移動遍歴処理 */

typedef	struct
{
	int 	ct;

	char	path[LN_path+1];
	char	name[LN_path+1];
}	dm_t;

#define	MAX_dm	64

dm_t	dm[MAX_dm];
int 	dm_num, dm_loc;

void	dm_init()
{
	int 	i;

	dm_num=0;
	dm_loc=0;

	for (i=0;i<MAX_dm;++i)
		dm[i].ct=0;
}

void	dm_set(const char *p,const char *s)
{
	int 	i,a,b;
	char	buf[LN_path+1];
	char	path[LN_path+1];

	realpath(p, path);
	strcpy(buf, s);
	if (*buf!='\0'&& buf[strlen(buf)-1]=='/')
		buf[strlen(buf)-1]='\0';

	for (i=0;i<dm_num;++i)
		{
		 if (strcmp(dm[i].path,path)==0)
		 	{
		 	 strcpy(dm[i].name,buf);
		 	 dm[i].ct=++dm_loc;
		 	 return ;
		 	}
		}

	if (dm_num<MAX_dm)
		{
		 strcpy(dm[dm_num].path,path);
		 strcpy(dm[dm_num].name,buf);
		 dm[dm_num].ct=++dm_loc;
		 ++dm_num;
		 return;
		}

	a=dm[0].ct;
	b=0;
	for (i=1;i<MAX_dm;++i)
		{
		 if (a>dm[i].ct)
		 	{
		 	 a=dm[i].ct;
		 	 b=i;
		 	}
		}

	strcpy(dm[b].path,path);
	strcpy(dm[b].name,buf);
	dm[b].ct=++dm_loc;
}

void	dm_get(const char *p,char *s)
{
	int 	i;
	char	path[LN_path+1];

	realpath(p, path);

	*s='\0';
	for(i=0;i<dm_num;++i)
		{
		 if (strcmp(dm[i].path,path)==0)
		 	{
		 	 strcpy(s,dm[i].name);
		 	 return;
		 	}
		}
	return;
}




void	fw_init(fw_t *fwp, const char *s,int a)
{
	char	path[LN_path+1];

	*fwp->match='\0';
	if (s!=NULL)
		strcpy(fwp->path, s); else
		{
		 getcwd(path, LN_path);
		 sprintf(fwp->path, "%s/", path);
		}

	fwp->flist.fitem=NULL;
	fwp->flist.n=0;
	fwp->findex=NULL;

	menu_iteminit(&fwp->menu);
	fwp->menu.title= fwp->path;

	fwp->menu.drp->sizex=(fwp->menu.drp->sizex+1)/2;
	if (a==1)
		fwp->menu.drp->x +=fwp->menu.drp->sizex;
}

void	eff_init(const char *s1, const char *s2)
{
	eff.wa=0;
	eff.wn=1;
	eff.sort[0]=SA_ext;
	eff.sort[1]=SA_fname;

	fw_init(&fw[0],s1,0);
	fw_init(&fw[1],s2,1);

	dm_init();
}



struct stat	*fw_getstat(fw_t *fwp,int a)
{
	if (a==-1)
		a=fwp->menu.cy+fwp->menu.sy;
//	min(a,fwp->flist.n);

	return &fwp->findex[a]->stat;
}


fitem_t	*fw_getfi(fw_t *fwp,int a)
{
	if (a==-1)
		a=fwp->menu.cy+fwp->menu.sy;
	return fwp->findex[a];
}

void	fwc_chdir(const char *s,bool f)
{
	char	path[LN_path+1];
	bool	uf;

	if (*s=='\0')
		return;

	if (strcmp(s,"..")==0)
		{
		 uf=TRUE;
		 sprintf(path,"%s..",fw_c.path);
		 reg_path(NULL, path,FALSE);
		}else
		{
		 uf=FALSE;
		 if (f)
		 	strcpy(path,s); else
		 	sprintf(path,"%s%s",fw_c.path,s);
		}

	reg_path(fw_c.path, path,FALSE);
	if (dir_isdir(path))
		{
		 dm_set(fw_c.path, fw_c.findex[fw_c.menu.cy+fw_c.menu.sy]->fn);
		 if (uf&& strcmp(fw_c.path,"/")!=0)
		 	dm_set(path,fw_c.path+strlen(path));

		 strcpy(fw_c.path,path);

		 if (!uf|| strcmp(fw_c.path,"/")==0)
		 	dm_get(path,fw_c.match); else
		 	{
		 	 strcpy(fw_c.match,fw_c.path+ strlen(path));
		 	 if (*fw_c.match!='\0' &&fw_c.match[strlen(fw_c.match)-1]=='/')
		 	 	fw_c.match[strlen(fw_c.match)-1]='\0';
		 	}

		 fw_c.df=TRUE;
		}
}

void	fw_match(fw_t *fwp)
{
	int 	i;

	i=fwp->flist.n-1;

	while (i>0 && strcmp(fwp->match, fwp->findex[i]->fn)!=0)
		--i;

	menu_csrmove(&fwp->menu, i);
}






fitem_t	*fitem_free(fitem_t *fip)
{
	fitem_t	*next;

	if (fip==NULL)
		return NULL;
	next=fip->next;

	free(fip->fn);
	free(fip);

	return next;
}

void	fw_free(fw_t *fwp)
{
	fitem_t	*fitem;

	menu_itemfin(&fwp->menu);

	free(fwp->findex);
	fitem=fwp->flist.fitem;
	while(fitem!=NULL)
		fitem=fitem_free(fitem);
	fwp->flist.fitem=NULL;
	fwp->flist.n=0;
}

fitem_t	*fitem_mk(const char *path, char *s)
{
	char	buf[LN_path+1], *p;
	fitem_t	*fitem;

	fitem=(fitem_t *)mem_alloc(sizeof(fitem_t));
	fitem->next=NULL;

	fitem->fn=s;

	p=dir_pext(s);
	if (p==NULL)
		fitem->e=s+strlen(s); else
		fitem->e=p;

	sprintf(buf,"%s/%s",path, s);
	if (strcmp(s,"..")==0)
		reg_path(NULL, buf, FALSE);
	stat(buf, &fitem->stat);

	return fitem;
}


#if 1

void	flist_set(flist_t *flp,const char *path)
{
	char	**p;
	fitem_t	*fitem;

	p=dir_glob(path, TRUE);

	fitem=  flp->fitem= fitem_mk(path, p[0]);
	flp->n= 1;

	while(p[flp->n]!=NULL)
		{
		 fitem->next= fitem_mk(path, p[flp->n]);
		 fitem= fitem->next;
		 ++flp->n;
		}
	free(p);
}

#else

void	flist_set(flist_t *flp,const char *path)
{
	DIR 			*dirp;
	struct	dirent	*dp;
	fitem_t			*fitem,*next;

	flp->fitem=NULL;
	flp->n=0;

	dirp=opendir(path);
	if (dirp==NULL)
		{
		 flp->fitem=fitem_mk(path,"..");
		 ++flp->n;
		 return;
		}

	for(;;)
		{
		 dp=readdir(dirp);
		 if (dp==NULL)
		 	break;
		 if (strcmp(dp->d_name, ".")==0)
		 	continue;

		 next=fitem_mk(path, dp->d_name);
		 if (flp->fitem==NULL)
		 	flp->fitem=next; else
		 	fitem->next=next;
		 fitem=next;
		 ++flp->n;
		}
	closedir(dirp);
}

#endif

static	int 	findex_comp(const void *x,const void *y)
{
	fitem_t	*fi_x,*fi_y;
	int 	i,j;

	fi_x=*(fitem_t **)x;
	fi_y=*(fitem_t **)y;

	if (strncmp(fi_x->fn,"..",2)==0)
		return -1;
	if (strncmp(fi_y->fn,"..",2)==0)
		return 1;

	if ((fi_x->stat.st_mode&S_IFMT)== S_IFDIR&& (fi_y->stat.st_mode&S_IFMT)!=S_IFDIR)
		return -1;
	if ((fi_x->stat.st_mode&S_IFMT)!= S_IFDIR&& (fi_y->stat.st_mode&S_IFMT)==S_IFDIR)
		return 1;

	if (*fi_x->fn=='.'&& *fi_y->fn!='.')
		return -1;
	if (*fi_x->fn!='.'&& *fi_y->fn=='.')
		return 1;

	for (i=0;i<2;++i)
		{
		 j=1;
		 switch(eff.sort[i])
		 	{
		  case SA_none:
		  	 return 0;
		  case SA_fname:
		  	 j=strcmp(fi_x->fn, fi_y->fn); //!! 厳密な判定を？
		  	 break;
		  case SA_ext:
		  	 j=strcmp(fi_x->e ,fi_y->e);
		  	 break;
		 	}
		 if (j!=0)
		 	return j;
		}
	return 0;
}


fitem_t	**findex_get(flist_t *flp)
{
	fitem_t	**findex;
	fitem_t	*fitem;
	int 	i;

	findex=(fitem_t **)mem_alloc(flp->n* sizeof(int *));

	fitem=flp->fitem;
	for (i=0;i<flp->n;++i)
		{
		 findex[i]=fitem;
		 fitem=fitem->next;
		}

	qsort(findex, i, sizeof(fitem_t **), findex_comp);

	return findex;
}

void	prt_kmsize(char *s,off_t n)
{
	const	char	*scale=" KMGT";

	if (n<(off_t)10*1000)
		{
		 sprintf(s," %4d", (int)n);
		 return;
		}

	while(n>=(off_t)10*1000 && *scale!='\0')
		{
		 n/=1000;
		 ++scale;
		}

	sprintf(s,"%4d%c", (int)n, *scale);
}

static	void	fw_item_proc(int a,mitem_t *mip,void *vp)
{
	char	buf[MAXLINESTR+1];
	char	*s,*p;
	fw_t	*fwp;

	fwp=vp;
	s=mip->str;

	strjfcpy(s, fwp->findex[a]->fn, MAXLINESTR, fwp->menu.drp->sizex-30);
	p=fwp->findex[a]->e;
	if (*p=='\0')
		strcat(s,"     "); else
		{
		 int 	m, n;

		 n=strlen(s);
		 m=fwp->findex[a]->e - fwp->findex[a]->fn-1;
		 if (m<n)
		 	memset(s+m, ' ', n-m);

		 strcat(s,".");
		 strjfcpy(s+strlen(s),p,MAXLINESTR,4); // !MAXLINESTR
		}

	strcat(s," ");
	if ((fwp->findex[a]->stat.st_mode&S_IFMT)==S_IFDIR)
		{
		 strcat(s,"<DIR>"); 
		 mip->cc=sysinfo.c_eff_dirc;
		 mip->nc=sysinfo.c_eff_dirn;
		}else
		{
		 prt_kmsize(buf, fwp->findex[a]->stat.st_size);
		 strcat(s, buf);

		 mip->cc=sysinfo.c_eff_normc;
		 mip->nc=sysinfo.c_eff_normn;
		}

	strftime(buf,15,"%y/%m/%d %R", localtime(&fwp->findex[a]->stat.st_mtime));
	sprintf(s+strlen(s), " %s",buf);
}

void	fw_make(fw_t *fwp)
{
	int 	sizex;

	sizex=-1;

	if (fwp->flist.fitem!=NULL)
		{
		 fw_free(fwp);
		 sizex=fwp->menu.drp->sizex;
		}

	dm_get(fwp->path, fwp->match);

	flist_set(&fwp->flist,fwp->path);
	fwp->findex=findex_get(&fwp->flist);

	fwp->marknum=0;
	fwp->markmax=0;
	fwp->mark=mem_alloc(sizeof(int)*fwp->flist.n);
	memset(fwp->mark,0,sizeof(int)*fwp->flist.n);

	fwp->menu.sy=0;
	fwp->menu.cy=0;

	menu_itemmake(&fwp->menu,fw_item_proc,fwp->flist.n,fwp);

	if (sizex!=-1)
		fwp->menu.drp->sizex=sizex;

	fw_match(fwp);
}


int 	fw_getmarkfirst(fw_t *fwp)
{
	int 	n,m;
	int 	i;

	m=-1;
	n=0;
	for (i=0;i<fwp->flist.n;++i)
		{
		 if (fwp->mark[i]!=0)
		 	{
		 	 if (n==0)
		 	 	{
		 	 	 n=fwp->mark[i];
		 	 	 m=i;
		 	 	} else
		 	 	{
		 	 	 if (n>fwp->mark[i])
		 	 	 	{
		 	 	 	 n=fwp->mark[i];
		 	 	 	 m=i;
		 	 	 	}
		 	 	}
		 	}
		}
	return m;
}

void	fw_chmark(fw_t *fwp,int a)
{
	int 	n;

	n= a!=-1? a: fwp->menu.cy+fwp->menu.sy;

	if (strcmp(fwp->findex[n]->fn,"..")==0)
		return;

	if (fwp->mark[n]==0)
		{
		 fwp->mark[n]=++fwp->markmax;
		 ++fwp->marknum;
		} else
		{
		 fwp->mark[n]=0;
		 --fwp->marknum;
		 if (fwp->marknum==0)
		 	fwp->markmax=0;
		}
}

void	fw_chmarkall(fw_t *fwp)
{
	int	i;

	if (fwp->marknum==0)
		{
		 for (i=0;i<fwp->flist.n;++i)
		 	{
		 	 if ((fw_getstat(fwp, i)->st_mode &S_IFMT)!=S_IFDIR)
		 	 	fw_chmark(fwp, i);
		 	}
		} else
		{
		 while((i=fw_getmarkfirst(fwp))!=-1)
		 	fw_chmark(fwp, i);
		}
}

/* file opration */

static	bool	select_readonly(fop_t *fop)
{
	int 	res;

	if (fop->pfm!=0)
		return fop->pfm==FP_force;

	system_msg("リードオンリーです。");

	res=menu_vselect(term_sizex()/2, term_sizey()/2
					, 4, "Y) 処理する", "N) 処理しない"
					,"A) 以降全て処理する", "K) 以降全て処理しない");


	RefreshMessage();
//	eff.df=TRUE;

	switch (res)
		{
	 case 2:
	 	 fop->pfm=FP_force;
	 case 0:
	 	 return TRUE;
	 case 3:
	 	 fop->pfm=FP_ignore;
	 	}
	return FALSE;
}


int 	fw_fop_file(const char *srcpath,const char *fn,struct stat *srcstp
			,const char *dstpath,fop_t *fop)
{
	char	dstfn[LN_path+1],srcfn[LN_path+1];
	char	tmp[LN_path+1];
	struct	stat	dstst;
	int 	res;

//fprintf(stderr,"fop_file ");

	sprintf(srcfn,"%s%s",srcpath,fn);
	if (dstpath!=NULL)
		sprintf(dstfn,"%s%s",dstpath,fn);

//fprintf(stderr,"%s -> %s\n",srcfn,dstfn);

	for (;;)
		{
		 if (dstpath!=NULL)
		 	{
		 	 if (stat(dstfn,&dstst)==0 || errno!=ENOENT)
		 	 	{
//fprintf(stderr,"%s(%x) -> %s(%x)\n"
//	,srcfn, srcstp->st_ino, dstfn, dstst.st_ino);

		 	 	 if (fop->om==0)
		 	 	 	{
					 fop->om=1+menu_vselect(term_sizex()/2, term_sizey()/2, 4
					 	, "O) 全て上書き", "U) 新しい日付を"
					 	, "N) 同名は複写しない", "R) 名前を変えて");

//		 	 	 	 eff.df=TRUE;
		 	 	 	}

		 	 	 switch(fop->om)
		 	 	 	{
		 	 	  case 0:
		 	 	  	 return FR_end;
		 	 	  case FO_owrite:
		 	 	  	 break;
		 	 	  case FO_update:
		 	 	  	 if (dstst.st_mtime < srcstp->st_mtime)
		 	 	  	 	break;
		 	 	  	 return FR_ok;
		 	 	  case FO_rename:
		 	 	  	 fop->om=0;
		 	 	  	 strcpy(tmp,fn);
		 	 	  	 if (GetS("Rename :",tmp)==ESCAPE)
		 	 	  	 	return FALSE;
		 	 	  	 sprintf(dstfn,"%s%s",dstpath,tmp);
		 	 	  	 continue;
		 	 	  case FO_none:
		 	 	  	 return FR_ok;
		 	 	 	}
		 	 	 if (access(dstfn,W_OK)<0&& !select_readonly(fop))
		 	 	 	return FR_ok;

		 	 	 unlink(dstfn);
		 	 	}
		 	}

		 res=fop->file_func(srcfn, srcstp, dstfn, fop);
		 if (res!=FR_err)
		 	return res;

		 res=menu_vselect(term_sizex()/2, term_sizey()/2, 3,
		 	"A) 中止", "R) 再試", "S) 継続");

//		 eff.df=TRUE;

		 switch(res)
		 	{
		  case -1:
		  case 0:
		  	 return FR_err;
		  case 1:
		  	 continue;
		  case 2:
		  	 return FR_ok;
		  	}
		}
}

int 	fw_fop_dir(const char *srcpath,const char *fn,struct stat *srcstp
			,const char *dstpath, fop_t *fop, bool wf)
{
	char	dstfn[LN_path+1],srcfn[LN_path+1];
	struct	stat	dstst;
	int 	res;
	bool	f;

	sprintf(srcfn,"%s%s",srcpath,fn);
	if (dstpath!=NULL)
		{
		 sprintf(dstfn,"%s%s",dstpath,fn);
		 if (stat(dstfn,&dstst)==0)
		 	{
		 	 if ((dstst.st_mode &S_IFMT)==S_IFDIR)
		 	 	{
		 	 	 chmod(dstfn,srcstp->st_mode);
		 	 	 touchfile(dstfn, srcstp->st_atime, srcstp->st_mtime);

		 	 	 return FR_ok;
		 	 	}

		 	 inkey_wait("ディレクトリと同名のファイルが存在します。");
		 	 return FR_err;
		 	}
//	 	 if (errno!=ENOENT)
		}

	f=TRUE;
	for (;;)
		{
		 res=fop->dir_func(srcfn, srcstp, dstfn, wf, fop);
		 if (res!=FR_err)
		 	return res;
		 res=menu_vselect(term_sizex()/2, term_sizey()/2, 3,
		 	"A) 中止", "R) 再試", "S) 継続");

//		 eff.df=TRUE;

		 switch(res)
		 	{
		  case -1:
		  case 0:
		  	 return FR_err;
		  case 1:
		  	 continue;
		  case 2:
		  	 return FR_ok;
		  	}
		}
}

int 	fw_fop_list(fitem_t **findex, size_t fi_nums
			,const char *srcpath, const char *dstpath
			,fop_t *fop)
{
	struct	stat	*srcstp;
	int 	res;
	int 	i;

	fitem_t	**fip;
	flist_t	fl;
	fitem_t	*fitem;
	char	srctmp[LN_path+1], dsttmp[LN_path+1];

//fprintf(stderr,"*%d %s -> %s\n",fi_nums, srcpath,dstpath);

	for (i=0;i<fi_nums;++i)
		{
		 srcstp=&findex[i]->stat;
//fprintf(stderr,"%03d:%s%s -> %s \n",i, srcpath,findex[i]->fn,dstpath);

		 if ((srcstp->st_mode& S_IFMT)!=S_IFDIR)
			res=fw_fop_file(srcpath, findex[i]->fn, srcstp, dstpath, fop); else
			{
			 if (strcmp(findex[i]->fn,".")==0|| strcmp(findex[i]->fn,"..")==0)
			 	continue;


			 sprintf(srctmp,"%s%s/",srcpath,findex[i]->fn);
			 if (dstpath!=NULL)
			 	sprintf(dsttmp,"%s%s/",dstpath,findex[i]->fn);
			 system_msg(srctmp);

			 res=fw_fop_dir(srcpath, findex[i]->fn, srcstp, dstpath, fop, TRUE);
			 if (res==FR_err)
			 	return FR_err;
			 if (res==FR_nonrec)
			 	continue;


		 	 flist_set(&fl,srctmp);
			 fip=findex_get(&fl);
//fprintf(stderr,"--- %s(%d) -> %s\n",srctmp, fl.n, dsttmp);

		 	 fw_fop_list(fip, fl.n, srctmp, dstpath==NULL?NULL:dsttmp
		 	 	, fop);

		 	 free(fip);
		 	 fitem=fl.fitem;
		 	 while(fitem!=NULL)
		 	 	fitem=fitem_free(fitem);


			 res=fw_fop_dir(srcpath, findex[i]->fn, srcstp, dstpath, fop, FALSE);
		 	}
		 if (res==FR_err|| res==FR_end)
		 	return res;
		}
	return FR_ok;
}


void	fw_fop(fw_t *srcfwp, const char *dstpath
			,char *title
			,int file_func(const char*,struct stat*,const char*,fop_t*)
			,int dir_func(const char*,struct stat*,const char*,bool,fop_t*))
{
	int 	mn;
	int 	n;
	int 	res;
	fop_t	fo;

	fo.title=title;
	fo.file_func=file_func;
	fo.dir_func=dir_func;
	fo.om=0;
	fo.pfm=0;

	mn=fw_c.menu.cy +fw_c.menu.sy;
	for (;;)
		{
		 fitem_t	*fip;

		 if (srcfwp->marknum==0)
		 	n=-1; else
		 	n=fw_getmarkfirst(srcfwp);

		 if (n!=-1)
		 	{
		 	 menu_csrmove(&fw_c.menu, n);
//		 	 dsp_regview
//			 dsp_allview();
		 	}

		 fip=fw_getfi(srcfwp,n);

		 res=fw_fop_list(&fip, 1, srcfwp->path, dstpath, &fo);
		 if (n!=-1)
		 	fw_chmark(srcfwp,n);

		 if (res==FR_err||res==FR_end)
		 	break;
		 if (srcfwp->marknum==0)
		 	break;
		}
	if (n!=-1)
		menu_csrmove(&fw_c.menu, mn);
}


static	int		cp_proc_dir(const char *src,struct stat *srcstp,const char *dst
			,bool wf,fop_t *fop)
{
	if (!wf)
		return FR_ok;

	if (mkdir(dst,0777)!=0)
		return FR_err;

	chmod(dst,srcstp->st_mode);
	touchfile(dst, srcstp->st_atime, srcstp->st_mtime);

	return FR_ok;
}

#define	MAX_cpbuf	4096

static	int		cp_proc_file(const char *src,struct stat *srcstp
								,const char *dst,fop_t *fop)
{
	FILE	*fpr,*fpw;
	bool	ef;

//fprintf(stderr,"cp*[%s]->[%s]\n",src,dst);

	fpr=fopen(src,"r");
	if (fpr==NULL)
		return FR_err;

	fpw=fopen(dst,"w");
	if (fpw==NULL)
		{
		 fclose(fpr);
		 return FR_err;
		}

	ef=FALSE;
	for (;;)
		{
		 char	buf[MAX_cpbuf];
		 int 	n,m;

		 n=fread(buf,1,MAX_cpbuf,fpr);
		 if (n==0)
		 	break;
		 m=0;
		 while(n-m>0)
		 	m=fwrite(buf+m,1,n-m,fpw);
		 if (m==0)
		 	{
		 	 ef=TRUE;
		 	 break;
		 	}
		}

	fclose(fpr);
	fclose(fpw);

	if (ef || ferror(fpr))
		{
		 unlink(dst);
		 return FR_err;
		}

	chmod(dst,srcstp->st_mode);
	touchfile(dst, srcstp->st_atime, srcstp->st_mtime);

	return FR_ok;
}

static	int		mv_proc_dir(const char *src,struct stat *srcstp,const char *dst
			,bool wf,fop_t* fop)
{
	if (!wf)
		{
		 rmdir(src);
		 return FR_ok;
		}

	if (rename(src,dst)==0)
		{
		 touchfile(dst, srcstp->st_atime, srcstp->st_mtime);
		 return FR_nonrec;
		}

	return cp_proc_dir(src,srcstp,dst,wf,fop);
}

static	int 	mv_proc_file(const char *src,struct stat *srcstp
								,const char *dst,fop_t *fop)
{
	int 	res;

	if (rename(src,dst)==0)
		return FR_ok;
	res=cp_proc_file(src,srcstp,dst,fop);
	if (res==FR_ok)
		unlink(src);
	return res;
}

static	int 	rm_proc_file(const char *src,struct stat *srcstp
									,const char *dst,fop_t *fop)
{
	if (access(src,W_OK)<0&& !select_readonly(fop))
		return FR_ok;

	if (unlink(src)==0)
		return FR_ok;
	return FR_err;
}

static	int 	rm_proc_dir(const char *src,struct stat *srcstp,const char *dst
			,bool wf,fop_t* fop)
{
	if (wf)
		return FR_ok;

	if (rmdir(src)==0)
		return FR_ok;
	return FR_err;
}

bool	fw_cpdest(char *s, fw_t *srcfwp, fw_t *dstfwp)
{
	char	srcpath[LN_path+1];

	strcpy(srcpath,srcfwp->path);
	reg_path(NULL, srcpath,TRUE);	// ??

	if (dstfwp!=NULL)
		{
		 strcpy(s,dstfwp->path);
		 reg_path(NULL, s,TRUE);
//fprintf(stderr,"dst [%s]\n",s);
		 if (strcmp(srcpath,s)!=0)	// !!自分の下かどうかチェックも行う
		 	return TRUE;
		}

	*s='\0';
	if (GetS("複写先 :",s)==ESCAPE)
		return FALSE;
//fprintf(stderr,"dst*[%s]\n",s);
	reg_path(srcpath, s,FALSE);

	if (strcmp(srcpath,s)==0)	// !!自分の下かどうかチェックも行う
		return FALSE;
//fprintf(stderr,"dst*[%s]\n",s);
	return mole_dir(s);

}

void	fw_copy(fw_t *srcfwp,fw_t *dstfwp)
{
	char	buf[LN_path+1];

	if (!fw_cpdest(buf,srcfwp,dstfwp))
		return;

	fw_fop(srcfwp, buf, "複写", cp_proc_file, cp_proc_dir);
	if (dstfwp!=NULL)
		fw_make(dstfwp);
}

void	fw_move(fw_t *srcfwp,fw_t *dstfwp)
{
	char	buf[LN_path+1];

	if (!fw_cpdest(buf,srcfwp,dstfwp))
		return;

	fw_fop(srcfwp, buf, "移動", mv_proc_file, mv_proc_dir);
	fw_make(srcfwp);

	if (dstfwp!=NULL)
		fw_make(dstfwp);
}

void	fw_remove(fw_t *srcfwp,fw_t *dstfwp)
{
	if (keysel_ynq("削除します。よろしいでしょうか？")!=TRUE)
		return;

	fw_fop(srcfwp, NULL,"削除", rm_proc_file, rm_proc_dir);
	fw_make(srcfwp);
}

void	fw_rename(fw_t *fwp)
{
	char	buf[LN_path+1], tmp[LN_path+1];
	char	*fn;

	fn=fw_c.findex[fw_c.menu.cy+fw_c.menu.sy]->fn;
	strcpy(tmp, fn);
	if (GetS("Rename :",tmp)==ESCAPE)
		return;

	getcwd(buf, LN_path);
	chdir(fwp->path);
	if (rename(fn, tmp)==0)
		dm_set(fwp->path,tmp);
	chdir(buf);

}

void	fw_mkdir(fw_t *fwp)
{
	char	buf[LN_path+1],tmp[LN_path+1];

	*tmp='\0';
	if (GetS("Mkdir :",tmp)==ESCAPE)
		return;
	sprintf(buf,"%s%s",fwp->path,tmp);
	mkdir(buf,0777);
}

char	*pval_parse(const char *s, char *t)
{
	char	c;
	char	*p, com[MAXLINESTR+1];

//fprintf(stderr,"\npval [%s]\n", s);

	c=' ';
	p=com;
	while (*s!='\0'&& *s!=c)
		{
		 if (*s=='(')
		 	{
		 	 c=')';
		 	 ++s;
		 	 continue;
		 	}
		 if (*s=='{')
		 	{
		 	 c='}';
		 	 ++s;
		 	 continue;
			}
		 *p++=*s++;
		}
	*p='\0';
	if (*s!='\0')
		++s;

	*t='\0';
//fprintf(stderr,"[%s]\n", com);
	switch(*com)
		{
//	 case 'm':
//		 if (fw_c.marknum>0)
	 case 'f':
		 strcpy(t, fw_c.findex[fw_c.menu.cy+fw_c.menu.sy]->fn);
	 	 break;
		}

	return (char *)s;
}


static	bool	eff_exec()
{
	char	buf[LN_path+1],*q;
	const	char	*p;
	bool	f_input,f_dread;


	*buf='\0';
	f_input=FALSE;
	f_dread=FALSE;
	if (keyf_numarg()>0)
		{
		 p=keyf_getarg(0);
		 q=buf;

//fprintf(stderr,"[%s]\n", p);

		 if (*p=='-')
		 	{
		 	 ++p;
		 	 while (*p!='\0'&&!isspace(*p))
		 	 	{
		 	 	 switch (tolower(*p))
		 	 	 	{
		 	 	  case 'i':
		 	 	  	 f_input=TRUE;
		 	 	  	 break;
		 	 	  case 'd':
		 	 	  	 f_dread=TRUE;
		 	 	  	 break;
		 	 	  	}
		 	 	 ++p;
		 	 	}

		 	 p=keyf_getarg(1);
		 	 if (p==NULL)
		 	 	p="";
		 	}

//fprintf(stderr,"[%s]\n", p);
		 while(*p!='\0')
		 	{
//fprintf(stderr,"<%c>", *p);
		 	 if (*p!='%')
		 	 	*q++=*p++; else
		 	 	{
		 	 	 p=pval_parse(p+1,q);
		 	 	 q+=strlen(q);
		 	 	}
		 	}
		}

	if (!f_input|| HisGets(buf, GETS_SHELL_MSG, SHELLS_SYSTEM) != NULL)
		CommandCom(buf);

	return f_dread;
}



bool	eff_filer(char *fn)
{
	int 	c;
	char	buf[LN_path+1], *p;
	bool	f;

//	term_cls();

	fw_make(&fw[0]);
	fw_make(&fw[1]);

	*fn='\0';

	if (eff.wn==1)
		dsp_regrm(fw[1].menu.drp);

	for (;;)
		{
		 dsp_allview();

		 term_csrh();
		 c=get_keyf(2);
		 switch(c)
		 	{
		  case -1:
		  	 continue;
		  case KF_EffCursorUp:
		  	 menu_csrmove(&fw_c.menu,fw_c.menu.cy +fw_c.menu.sy -1);
		  	 continue;

		  case KF_EffMarkChangeAll:
		  	 fw_chmarkall(&fw[eff.wa]);
		  	 continue;
		  case KF_EffMarkChange:
		  	 fw_chmark(&fw[eff.wa],-1);

		  case KF_EffCursorDown:
		  	 menu_csrmove(&fw_c.menu,fw_c.menu.cy +fw_c.menu.sy +1);
		  	 continue;
		  case KF_EffPageUp:
		  	 menu_csrmove(&fw_c.menu,fw_c.menu.cy +fw_c.menu.sy -(fw_c.menu.drp->sizey-1));
		  	 continue;
		  case KF_EffPageDown:
		  	 menu_csrmove(&fw_c.menu,fw_c.menu.cy +fw_c.menu.sy +(fw_c.menu.drp->sizey-1));
		  	 continue;
		  case KF_EffRollUp:
		  	 menu_csrmove(&fw_c.menu,fw_c.menu.cy +fw_c.menu.sy -fw_c.menu.drp->sizey/4);
		  	 continue;
		  case KF_EffRollDown:
		  	 menu_csrmove(&fw_c.menu,fw_c.menu.cy +fw_c.menu.sy +fw_c.menu.drp->sizey/4);
		  	 continue;
		  default:
		  	 if (c&KF_normalcode)
		  	 	{
		  	 	 c&= ~KF_normalcode;
		  	 	 if (isupper(c&0xff))
		  	 	 	menu_csrnext(&fw_c.menu, c);
		  	 	}
		  	 continue;

		  case KF_EffExec:
		  	 if (!eff_exec())
		  	 	continue;

		  case KF_EffReRead:
		  	 dm_set(fw_c.path, fw_c.findex[fw_c.menu.cy+fw_c.menu.sy]->fn);

		  	 fw_make(&fw[eff.wa]);
		  	 term_cls();
		  	 continue;

		  case KF_EffRename:
		  	 fw_rename(&fw[eff.wa]);
		  	 fw_make(&fw[eff.wa]);
		  	 continue;
		  case KF_EffMkdir:
		  	 fw_mkdir(&fw[eff.wa]);
		  	 fw_make(&fw[eff.wa]);
		  	 continue;

		  case KF_EffWindowChange:
		  	 if (eff.wn==2)
		  	 	{
		  	 	 fw_c.menu.df=TRUE;
		  	 	 eff.wa= (eff.wa==0)?1:0;
		  	 	 fw_c.menu.df=FALSE;
		  	 	}
		  	 continue;
		  case KF_EffWindowNumChange:
		  	 if (eff.wn==1)
		  	 	{
		  	 	 eff.wn=2;
		  	 	 fw[1].menu.df=TRUE;
		  	 	 dsp_regadd(fw[1].menu.drp);
		  	 	}else
		  	 	{
		 		 eff.wa=0;
		  	 	 fw_c.menu.df=FALSE;
		  	 	 eff.wn=1;
		  	 	 dsp_regrm(fw[1].menu.drp);

				 term_cls();
		  	 	}
		  	 continue;

		  case KF_EffFileCp:
		  	 fw_copy(&fw[eff.wa], eff.wn==2? &fw[eff.wa==0?1:0]: NULL);
		  	 continue;
		  case KF_EffFileMv:
		  	 fw_move(&fw[eff.wa], eff.wn==2? &fw[eff.wa==0?1:0]: NULL);
		  	 continue;
		  case KF_EffFileRm:
		  	 fw_remove(&fw[eff.wa],NULL);
		  	 continue;

		  case KF_EffChangeDir:
		  	 if (keyf_numarg()>0)
		  	 	{
		  	 	 fwc_chdir(keyf_getarg(0), TRUE);
		  	 	 fw_make(&fw[eff.wa]);
//				 term_cls();
		  	 	 continue;
		  	 	}
		  	 *buf='\0';
		  	 if (GetS("Change Dir: ",buf))
		  	 	{
		  	 	 fwc_chdir(buf,TRUE);
		  	 	 fw_make(&fw[eff.wa]);
//				 term_cls();
		  	 	}
		  	 continue;

		  case KF_EffReturn:
		 	 p=fw_c.findex[fw_c.menu.cy+fw_c.menu.sy]->fn;

		  	 if ((fw_c.findex[fw_c.menu.cy+fw_c.menu.sy]->stat.st_mode &S_IFMT)
		  	 		==S_IFDIR)
		  	 	{
		  	 	 fwc_chdir(p, FALSE);
		  	 	 fw_make(&fw[eff.wa]);
//				 term_cls();
		  	 	 continue;
		  	 	}
		  	 dm_set(fw_c.path, p);
		  	 sprintf(fn,"%s%s",fw_c.path, p);
		  	 f=TRUE;
		  	 break;
		  case KF_EffEscape:
		  	 dm_set(fw_c.path, fw_c.findex[fw_c.menu.cy+fw_c.menu.sy]->fn);
		  	 f=FALSE;
		  	 break;
		  	}

		 break;
		}

	fw_free(&fw[0]);
	fw_free(&fw[1]);

	dsp_regrm(fw[0].menu.drp);
	dsp_regrm(fw[1].menu.drp);
//	dsp_regfin(fw[0].menu.drp);
//	dsp_regfin(fw[1].menu.drp);


	return f;
}


/*
	2000/03/11 by Mia	add
		returns if filer should be used.
		this function is not static because this would be useful for others.
*/
bool	need_filer( const char* pszFilename )
{
	if (*pszFilename=='\0')
		return TRUE;

	/*	if filename contains '*' or '?', yes.
		Although shell expand this characters,
		ESC-o doesn't.
	*/
	if (strchr(pszFilename, '*') != NULL ||
		strchr(pszFilename, '?') != NULL)
		return TRUE ;

	/*	if it is a directory, yes. */
	return dir_isdir(pszFilename);
}

