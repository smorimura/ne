/*--------------------------------------------------------------------
	nxeditor
			FILE NAME:file.c
			Programed by : I.Neva
			R & D  ADVANCED SYSTEMS. IMAGING PRODUCTS.
			1992.06.01

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 
--------------------------------------------------------------------*/
#include	"ed.h"
#include	"function.h"
#include	<sys/stat.h>
#include	<sys/file.h>

#ifdef	HAVE_SYS_FILE_H
#	include	<sys/file.h>
#endif

void	FileStartInit(bool f) // !! list clear flag 
{
	edbuf[CurrentFileNo].cf = FALSE;
	BlockInit();

// cursor 
	csrle.cx=0;
	csrle.lx=0;
	csrle.sx=0;
	csrle.dsize=GetColWidth()-NumWidth;
	csrle.size=MAXEDITLINE;
	csr_fix();

	csrse.cy=GetMinRow();
	csrse.ly=GetTopNumber();
	csrse.gf=FALSE;
	csrse.bytes=0;

	if (f)
		lists_clear();

	csrse.ed=GetTop()->next;
}

void	edbuf_init()
{
	int 	i;

	for (i=0; i<MAX_edbuf; i++)
		{
		 *edbuf[i].path='\0';
		 edbuf[i].pm=FALSE;
		}

	for (CurrentFileNo= MAX_edfiles
			; CurrentFileNo< MAX_edbuf; ++CurrentFileNo)
		FileStartInit(FALSE);
}


#ifndef	LOCK_SH
#define	LOCK_SH	1
#define	LOCK_EX	2
#define	LOCK_NB	4
#define	LOCK_UN	8
#endif

static	bool	edbuf_lock(bool func(FILE *, const char*), const char *fn)
{
	char	buf[LN_path+1];
	FILE	*fp;
	int 	i, a;
	bool	f;

	sysinfo_path(buf, "ne_lock");
	fp=fopen(buf, "r+");
	if (fp==NULL)
		{
		 fp=fopen(buf, "w+");
		 if (fp==NULL)
		 	{
			 inkey_wait("lock �ե����뤬 open ����ޤ���");
			 return TRUE;
			}
		}

	for (i=0;;++i)
		{
		 a=flock(fileno(fp), LOCK_EX | LOCK_NB);
		 if (a==0)
		 	break;
		 if (i>=10)
		 	{
		 	 fclose(fp);
		 	 inkey_wait("lock �ե����뤬 lock ����Ƥ��ޤ���");
		 	 return TRUE;
		 	}
		}

	f=func(fp, fn);

	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return f;
}


static	bool	edbuf_rm_func(FILE *fp, const char *fn)
{
	char	buf[LN_path+1];
	char	*p ,*q;
	long	n;

	p=NULL;
	n=0;
	while(fgets(buf, LN_path, fp))
		{
		 if (*buf!='\0' && buf[strlen(buf)-1]=='\n')
		 	buf[strlen(buf)-1]='\0';

		 if (strcmp(fn, buf)==0)
		 	continue;

		 n+=strlen(buf)+1;
		 q=(char *)mem_alloc(n+1);
		 if (p==NULL)
		 	sprintf(q, "%s\n",buf); else
		 	sprintf(q, "%s%s\n", p, buf);
		 free(p);
		 p=q;
		}

	rewind(fp);
	if (n>0)
		{
		 fputs(p, fp);
		 fflush(fp);
		}
	ftruncate(fileno(fp), n);

	return n>0;
}

void	edbuf_rm(int n)
{
	char	buf[LN_path+1];

	if (!edbuf_lock(edbuf_rm_func, edbuf[n].path))
		{
		 sysinfo_path(buf, "ne_lock");
		 unlink(buf);
		}
	*edbuf[n].path = '\0';
}



static	bool	edbuf_add_func(FILE *fp, const char *fn)
{
	char	buf[LN_path+1];

	while(fgets(buf, LN_path, fp))
		{
		 if (*buf!='\0' && buf[strlen(buf)-1]=='\n')
		 	buf[strlen(buf)-1]='\0';

		 if (strcmp(fn, buf)==0)
		 	return FALSE;
		}

	fprintf(fp, "%s\n", fn);
	return TRUE;
}

bool	edbuf_add(const char *fn)
{
	int 	i;

	for (i=0; i<MAX_edfiles; i++)
		{
		 if (*edbuf[i].path=='\0')
		 	break;
		}

	if (i>=MAX_edfiles)
		{
		 inkey_wait(DONT_OPEN_MORE_FILE_MSG);
		 return FALSE;
		}

	if (!edbuf_lock(edbuf_add_func, fn))
		{
		 inkey_wait("���˥����ץ󤵤�Ƥ��ޤ���");
		 return FALSE;
		}

	strcpy(edbuf[i].path, fn);
	BackFileNo=CurrentFileNo;
	CurrentFileNo=i;
	return TRUE;
}

bool	edbuf_mv(int n, const char *fn)
{
	edbuf_lock(edbuf_rm_func, edbuf[n].path);
	if (!edbuf_lock(edbuf_add_func, fn))
		{
		 inkey_wait("���˥����ץ󤵤�Ƥ��ޤ���");
		 if (!edbuf_lock(edbuf_add_func, edbuf[n].path))
		 	inkey_wait("���ե�����⥪���ץ󤵤�Ƥ��ޤ���");
		 return FALSE;
		}

	strcpy(edbuf[n].path, fn);
	return TRUE;
}





bool	CheckFileAccess(const char *fn)
{
	struct stat	sbuf;
	bool	f;

	if (fn!=NULL && strcmp(fn, edbuf[CurrentFileNo].path)!=0)
		return FALSE;

	if (edbuf[CurrentFileNo].ct != -1)
		{
		 f=stat(edbuf[CurrentFileNo].path, &sbuf);
		 if (f!=-1 && sbuf.st_ctime != edbuf[CurrentFileNo].ct)
		 	return TRUE;
		}
	return FALSE;
}

bool	file_change(int n)
{
	/*	2000/03/08 by Mia	fix
		edbuf[n].path=='\0' cannot be true.
	*/
	if (n<0 || n>MAX_edfiles || *edbuf[n].path=='\0')
		return FALSE;

	BackFileNo = CurrentFileNo;
	CurrentFileNo = n;
	if (CheckFileAccess(NULL))
		system_msg(TARGET_FILE_CHANGED_MSG);
	return TRUE;
}


void	SetFileChangeFlag()
{
	edbuf[CurrentFileNo].cf = TRUE;
}

void	ResetFileChangeFlag()
{
	edbuf[CurrentFileNo].cf = FALSE;
}

int 	CheckFileOpened(const char *fn)
{
	int 	i;

	for (i=0 ;i<MAX_edfiles; i++) 
		{
		 if (*edbuf[i].path=='\0')
		 	continue;

		 if (strcmp(edbuf[i].path, fn) == 0) 
		 	return i;
		}
	return -1;
}




int 	FindOutNextFile(int no)
{
	int i;

	for (i=no; i<MAX_edfiles; i++) 
	{
		if (*edbuf[i].path != '\0' && i != CurrentFileNo)
			return i;
	}
	for (i=0; i<no; i++) 
	{
		if (*edbuf[i].path != '\0' && i != CurrentFileNo)
			return i;
	}
	return -1;
}

int 	GetBackFile(int n)
{
	if (BackFileNo!=n && *edbuf[BackFileNo].path != '\0')
		return BackFileNo;
	return FindOutNextFile(n);
}









void	filesave_proc(const char *s,FILE *fp)
{
	bool	f;
	char	buf[MAXEDITLINE+1];
	char	f_buf[MAXEDITLINE+1];
	char	*rm_table[]={"\n","\r\n","\r"};

	if (s==NULL)
		return;
	strjncpy(buf, s, MAXEDITLINE);
	if (*s=='\0'|| s[strlen(s)-1]!='\n')
		f=FALSE; else
		{
		 buf[strlen(s)-1]='\0';
		 f=TRUE;
		}

	fputs(kanji_fromeuc(f_buf,MAXEDITLINE,buf,edbuf[CurrentFileNo].kc) ,fp);
	if (f)
		fputs(rm_table[edbuf[CurrentFileNo].rm] ,fp);

//	kanji_fputs(s,fp,edbuf[CurrentFileNo].kc,edbuf[CurrentFileNo].rm);
}

int 	filesave(char *filename,bool f)
{
	FILE	*fp;
	int 	res;

	/*	2000.03.08 by Mia	fix
		If strlen(filename) == LN_path,
		strcat(backpath, ".bak") may fail.
		This should be fixed although a pathological case.
	*/
	char backpath[LN_path+4+1];
	char buffer[MAXEDITLINE+1];


	if (!f)
		{
		 if (!edbuf[CurrentFileNo].cf)
		 	return TRUE;

		 term_bell();
		 res = keysel_yneq(THIS_FILE_CHANGED_MSG);

		 if (res==ESCAPE)
		 	return FALSE;
		 if (!res)
		 	return TRUE;
		}

	if (access(filename, F_OK) == 0 && access(filename, W_OK) == -1) 
		{
		 CrtDrawAll();
		 inkey_wait(DONT_WRITE_FILE_MSG);
		 return FALSE;
		}


	if (CheckFileAccess(filename))
		{
		 term_bell();
		 res = keysel_yneq(SAVE_FILE_CHANGED_MSG);

		 if (res==ESCAPE|| !res)
		 	return FALSE;
		}


	if (sysinfo.backupf && access(filename, F_OK)==0)
		{
		 strcpy(backpath,filename);
		 strcat(backpath,".bak");
		 unlink(backpath);
		 rename(filename, backpath);
		}

	fp=fopen(filename,"w"); 
	if (fp==NULL) 
		{
		 CrtDrawAll();
		 inkey_wait(DONT_WRITE_FILE_MSG);
		 return FALSE;
		}

	sprintf(buffer, "%s %s .. ", SAVEING_MSG, filename);
	lists_proc(filesave_proc,fp,GetTopNumber(),GetLastNumber());
	fclose(fp);
	ResetFileChangeFlag();

	sprintf(buffer, "%s %s .. ", SAVESUCCESS_MSG, filename);
	system_msg(buffer);

	return TRUE;
}


void	*file_open_proc(char *s,kinfo_t *kip)
{
	char	buf[MAXEDITLINE+1];
	bool	f;

	f=file_gets(buf, MAXEDITLINE, kip->fp, &kip->n_cr, &kip->n_lf);
	kanji_toeuc(s, MAXEDITLINE, buf, kip->kc==KC_sjis, &kip->jm);
//fprintf(stderr,"[%s]%d\n",s,f);
	return f!=-1?kip:NULL;
}

void	*file_new_proc(char *s,void *vp)
{
	*s='\0';
	return NULL;
}


int 	fileopen(char *filename)
{
	FILE	*fp;
	char	buf[MAXEDITLINE+1];
	struct stat	sbuf;
	kinfo_t	ki;
	int 	n,nx;
	bool	sf;

	sf=stat(filename, &sbuf);
	if (!sf&& (sbuf.st_mode&S_IFMT)!= S_IFREG)
		return FALSE;	/* �쥮��顼�ե�����ǤϤʤ���*/

	fp=fopen(filename,"r");
	if (fp==NULL)
		{
		 if (access(filename,F_OK)==-1) /* �����ե����� */
		 	{
		 	 lists_add(file_new_proc,"");
		 	 edbuf[CurrentFileNo].ct = -1;

		 	 sprintf(buf, "%s [ %s ]", NEWFILE_MSG, filename);
		 	 system_msg(buf);
		 	 csr_lenew();
		 	 return TRUE;
		 	}
		 if (access(filename, R_OK) == -1) 
		 	 inkey_wait(PERMISSION_MSG); else
		 	 inkey_wait("�����ʥ��顼");
		 return FALSE;
		}

	edbuf[CurrentFileNo].ct= sf? sbuf.st_ctime: -1;
	sprintf(buf,"%s  %s ",READING_MSG,filename);
	system_msg(buf);


	edbuf[CurrentFileNo].kc=file_knjchk(fp);
	rewind(fp);

	ki.fp=fp;
	ki.kc=edbuf[CurrentFileNo].kc;
	ki.n_cr=0;
	ki.n_lf=0;
	ki.jm=JM_ank;
	lists_add(file_open_proc,&ki);


/* CR/LF�⡼�ɤγ��� */
	n=max(ki.n_cr,ki.n_lf);
	if (n==0)
		edbuf[CurrentFileNo].rm=0; else
		{
		 nx=n>10? 10:1;

		 if (n/nx>ki.n_cr)
		 	edbuf[CurrentFileNo].rm=RM_lf; else
		 	edbuf[CurrentFileNo].rm=n/nx>ki.n_lf?RM_cr:RM_crlf;
		}

	csr_lenew();
/*	SetPermissionFlg(filename);*/

	RefreshMessage();

	return TRUE;
}

bool 	file_insert(char *filename)
{
	FILE	*fp;
	kinfo_t	ki;


	fp=fopen(filename,"r");
	if (fp==NULL)
		return FALSE;

	system_msg(WAITING_MSG);

	ki.kc=file_knjchk(fp);
	ki.fp=fp;
	ki.n_cr=0;
	ki.n_lf=0;
	ki.jm=JM_ank;
	rewind(fp);

	lists_add(file_open_proc,&ki);

	csr_lenew();
/*	SetPermissionFlg(filename);*/
	return TRUE;
}

bool	RenameFile(int current_no,const char *s)
{
	int 	i;
	char	fn[MAXEDITLINE+1];
	struct stat	sbuf;

	if (s!=NULL&& *s!='\0')
		strcpy(fn,s); else
		{
		 strcpy(fn,edbuf[current_no].path);
		 if (HisGets(fn, RENAME_MSG, FOPEN_SYSTEM) == NULL)
		 	return FALSE;
		 if (*fn=='\0')
		 	return FALSE;
		}
	reg_pf(NULL, fn,FALSE);

	for (i=0; i<MAX_edfiles; i++)
		{
		 if (*edbuf[i].path=='\0')
		 	continue;

		 if (strcmp(edbuf[i].path, fn) == 0) 
		 	{
		 	 if (!keysel_ynq(THIS_FILE_OPENED_MSG)) 
		 	 	return FALSE;
		 	 break;
		 	}
		}

	if (access(fn, F_OK)!=-1&& !keysel_ynq(FILE_EXIST_MSG)) 
		return FALSE;

	if (edbuf_mv(current_no, fn))
		return FALSE;

	SetFileChangeFlag();
	edbuf[current_no].ct= stat(edbuf[current_no].path,&sbuf)==-1?
			-1: sbuf.st_ctime;
	return TRUE;
}


bool	FileOpenOp(const char *path)
{
	int 		n;
	char		pf[LN_path+1];

	if (*path=='\0')
		return FALSE;
	strcpy(pf,path);
	reg_pf(NULL, pf,FALSE);

	n=CheckFileOpened(pf);
	if (n!=-1)
		{
		/*	2000/03/08 by Mia	upd
			Do not call file_change
			when requested file has been already opened and is current file
			so that BackFileNo not be changed.
		*/
		 file_change(n);
		 return FALSE;
		}

	if (!edbuf_add(pf))
		return FALSE;

	FileStartInit(TRUE); //!!?? �ʤ������ˤ��줬ɬ�ס�

	if (fileopen(edbuf[CurrentFileNo].path))
		return TRUE;

	edbuf_rm(CurrentFileNo);
	lists_clear();
	CurrentFileNo=BackFileNo;

	return FALSE;
}



/*-----------------------------------------------------------------------------
	ESC-O handler.

	2000/03/11 by Mia	upd
		hack up filer support.
*/
SHELL	bool	op_file_open()				/* ^[O */
{
	char	fname[LN_path+1];

	*fname='\0';
	if (HisGets(fname, OPEN_MSG, FOPEN_SYSTEM)==NULL)
		return FALSE;

	fname[LN_path] = '\0';
	if (need_filer( fname ))
		eff_filer(fname);

	if (*fname=='\0')
		{
		 CrtDrawAll();
		 return FALSE;
		}

//	system_msg(WAITING_MSG);
	return FileOpenOp(fname);
}

SHELL	bool	op_file_insert()				/* ^[I */
{
	char	fname[MAXEDITLINE+1];

	*fname='\0';
	if (HisGets(fname, INS_MSG, FOPEN_SYSTEM)==NULL)
		return FALSE;

	fname[LN_path] = '\0';
	if (need_filer( fname ))
		eff_filer(fname);

	if (*fname=='\0')
		{
		 CrtDrawAll();
		 return FALSE;
		}

//	system_msg(WAITING_MSG);
	file_insert(fname);
	SetFileChangeFlag();
	return TRUE;
}



SHELL	void	op_file_save()		/* ^[S */
{
	struct stat sbuf;
	int 	cTime;
	char	fn[LN_path+1];

	csr_leupdate();

//	if (!RenameFile(CurrentFileNo, NULL))
//		return;
	strcpy(fn,edbuf[CurrentFileNo].path);
	if (HisGets(fn, RENAME_MSG, FOPEN_SYSTEM) == NULL)
		return;
	if (*fn=='\0')
		return;

	if (!filesave(fn,TRUE))
		return;

	if (strcmp(fn,edbuf[CurrentFileNo].path)==0)
		{
		 cTime= stat(edbuf[CurrentFileNo].path, &sbuf);
		 edbuf[CurrentFileNo].ct = cTime==-1?-1:sbuf.st_ctime;
		}
}

SHELL	void	op_file_profile()					/* ^[W */
{
	csr_leupdate();
	profile_write();

	system_msg(SAVE_PROFILE_MSG);
}

bool	fileclose(int n)
{
	int 	m;

	csr_leupdate();


	m=CurrentFileNo;
	CurrentFileNo=n;

	if (edbuf[n].cf &&!filesave(edbuf[n].path,FALSE))
		return FALSE;

//	CrtDrawAll();
	edbuf_rm(n);
	lists_clear();

	CurrentFileNo=m;

	return TRUE;
}

SHELL	bool	op_file_close()					/* ^[C */
{
	int 	n;

	if (!fileclose(CurrentFileNo))
		return FALSE;

	if (!file_change(BackFileNo))
		{
		 n=FindOutNextFile(CurrentFileNo);
		 if (n==-1 || !file_change(n))
		 	ne_fin();
		}
	BackFileNo = FindOutNextFile(CurrentFileNo);
	return TRUE;
}

SHELL	void	op_file_aclose()
{
	 int i;

	for (i=0; i<MAX_edfiles; i++)
		{
		 if (*edbuf[i].path == '\0')
		 	continue;
		 CurrentFileNo=i;
		 CrtDrawAll();
		 if (!fileclose(i))
		 	return;
		}
	ne_fin();
}

SHELL	void	op_file_quit()
{
	int 	i;

	op_file_profile();

	for (i=0; i<MAX_edfiles; i++)
		{
		 if (*edbuf[i].path == '\0')
		 	continue;
		 CurrentFileNo=i;
		 edbuf_rm(i);
		 lists_clear();
		}
	ne_fin();
}


void 	op_file_undo()	/* �Խ�undo */
{
	 long lineOffset;
	char	pf[LN_path+1];
	bool	res;

	csr_leupdate();

	if (!edbuf[CurrentFileNo].cf)
		return;
	term_bell();
	res=keysel_yneq(ARE_YOU_SURE_MSG);
	if (res==ESCAPE|| !res)
		return;

	strcpy(pf, edbuf[CurrentFileNo].path);
	lineOffset = GetLineOffset();

	FileStartInit(TRUE);

	if (!fileopen(pf))
		{
		 edbuf_rm(CurrentFileNo);
		 lists_clear();
		 CurrentFileNo=BackFileNo;

		 system_msg(TARGET_FILE_PERM_MSG);
		}
	

	csr_setly(lineOffset);
}

static int file_menu_res = 0;
void	op_menu_file()
{
	 int res;

	res=menu_vselect(-1, -1, 10, MENU_OPEN_MSG, MENU_CLOSE_MSG, MENU_SAVE_MSG
				,MENU_SAVEAS_MSG,MENU_CLOSE_AF_MSG,MENU_SAVE_PROFILE_MSG
				,MENU_RENAME_MSG,MENU_REFRESH_CF_MSG,MENU_ESCAPE_SHELL_MSG,
				MENU_INSERT_OUTPUT_MSG);

		switch(res) {
		case 0:
			op_file_open();
			break;
		case 1:
			op_file_close();
			break;
		case 2:
			op_file_save();
			break;
		case 3:
// �ե�����̾����ꤷ��save��
			break;
		case 4:
			op_file_aclose();
			break;
		case 5:
			op_file_profile();
			break;
		case 6:
			RenameFile(CurrentFileNo, NULL);
			break;
		case 7:
			op_file_undo();
			break;
		case 8:
			op_misc_exec("");
			break;
		/*	exec and get stdout/stderr */
		case 9 :
			op_misc_insert_output( ) ;
			break ;

		default:	//!!
			return;
		}
		file_menu_res = res;
}

SHELL	void	op_file_copen()
{
	int 	n,m;

	n=CurrentFileNo;
	if (!op_file_open())
		return;

	m=CurrentFileNo;
	CurrentFileNo=n;
	CrtDrawAll();

	if (op_file_close() &&CurrentFileNo!=m)
		{
		 CurrentFileNo=m;
		 CrtDrawAll();
		}
}

void 	op_file_toggle()
{
	 int tmpNo;

	if ((tmpNo = GetBackFile(CurrentFileNo)) != -1)
		file_change(tmpNo);
}

char	tmpbuff[MAX_edbuf][MAXLINESTR+1];
/*void	file_select_proc(int a,mitem_t *mip,void *vp)
{
	strcpy(mip->str,tmpbuff[a]);
}
}
*/

int 	SelectFileMenu()
{
	int 	i,j;
	int 	m;
	int 	res;
	menu_t	menu;

	for (i = 0, j = 0, m = 0; i < MAX_edfiles; i++) 
	{
		if (*edbuf[i].path=='\0')
			continue;
		if (i < CurrentFileNo)
			m++;
		sprintf(tmpbuff[j],"%-.*s %1s ",GetColWidth()-6,edbuf[i].path
				,(edbuf[i].cf ? "*":""));
		j++;
	}

	menu_iteminit(&menu);
	menu_itemmakelists(&menu, MAXLINESTR+1, j, (char *)tmpbuff);

	res=menu_select(&menu);

	CrtDrawAll();

	return res==-1? -1:res;
}

void	op_file_select()
{
	 int i;
	 int j;
	 int file_no;

	if ((file_no = SelectFileMenu()) != -1) 
	{
		for (i=j=0; i<MAX_edfiles; i++) 
		{
			if (*edbuf[i].path != '\0') 
			{
				if (j== file_no)
					break;
				j++;
			}
		}
		file_change(i);
	}
}

SHELL	void	op_file_rename()
{
	RenameFile(CurrentFileNo,NULL);
}
