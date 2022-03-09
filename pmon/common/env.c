/* $Id: env.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

/*
 * Copyright (c) 2001-2002 Opsycon AB  (www.opsycon.se / www.opsycon.com)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Opsycon AB, Sweden.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#include <stdio.h>
#include <termio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _KERNEL
#undef _KERNEL
#include <sys/ioctl.h>
#define _KERNEL
#else
#include <sys/ioctl.h>
#endif
#include <pmon.h>
#include <pmonenv.h>

#include "cmd_more.h"

#include "../cmds/bootparam.h"

static int	envinited = 0;

static struct stdenv {
    char           *name;
    char           *init;
    char           *values;
    int		   (*chgfunc) __P((char *, char *));
} stdenvtab[] = {
    {"brkcmd", "l -r @cpc 1", 0},
#ifdef HAVE_QUAD
    {"datasize", "-b", "-b -h -w -d"},
#else
    {"datasize", "-b", "-b -h -w"},
#endif
    {"dlecho", "off", "off on lfeed"},
    {"dlproto", "none", "none XonXoff EtxAck"},
#ifdef INET
    {"bootp",	"no", "no sec pri save"},
#endif
    {"hostport", "tty0", 0},
    {"inalpha", "hex", "hex symbol"},
    {"inbase", INBASE_DEFAULT, INBASE_STRING},
#if NCMD_MORE > 0
    {"moresz", "10", 0, chg_moresz},
#endif
    {"prompt", "PMON> ", 0},
    {"regstyle", "sw", "hw sw"},
#ifdef HAVE_QUAD
    {"regsize", "32", "32 64"},
#endif
    {"rptcmd", "trace", "off on trace"},
    {"trabort", "^K", 0},
    {"ulcr", "cr", "cr lf crlf"},
    {"uleof", "%", 0},
#ifdef PMON_DEBUGGER /* XXX: Patrik temp */
    {"validpc", "_ftext etext", 0, chg_validpc},
#endif
    {"heaptop", SETCLIENTPC, 0, chg_heaptop},
    {"showsym", "yes", "no yes"},
    {"fpfmt", "both", "both double single none"},
    {"fpdis", "yes", "no yes"},
#if defined(TGT_DEFENV)
    TGT_DEFENV,
#endif
    {0}
};

struct envpair envvar[NVAR];

static int _matchval __P((const struct stdenv *, char *));
static const struct stdenv *getstdenv __P((const char *));
static int _setenv __P((char *, char *));

static int 
_matchval (sp, value)
    const struct stdenv *sp;
    char *value;
{
    char *t, wrd[20];
    int j;

    for (j = 0, t = sp->values; ; j++) {
	t = getword (wrd, t);
	if (t == 0)
	    return (-2);
	if (striequ (wrd, value))
	    return (j);
    }
}


static const struct stdenv *
getstdenv (name)
    const char *name;
{
    const struct stdenv *sp;
    for (sp = stdenvtab; sp->name; sp++)
	if (striequ (name, sp->name))
	    return sp;
    return 0;
}


static int
_setenv (name, value)
    char *name, *value;
{
    struct envpair *ep;
    struct envpair *bp = 0;
    const struct stdenv *sp;

    if ((sp = getstdenv (name)) != 0) { //使用名字查询，是否在标准环境变量表中，不存在返回0
	if (sp-> chgfunc && !(*sp->chgfunc) (name, value)) //如果在标准表中，看这个是否又对应的函数，有则调用该函数
	    return 0;
	if (sp->values && _matchval (sp, value) < 0 && envinited) {//如果存在，则看原来的值是不是不存在
	    printf ("%s: bad %s value, try [%s]\n", value, name, sp->values);  //不存在则返回0
	    return 0;
	}
    }

    for (ep = envvar; ep < &envvar[NVAR]; ep++) {  //在环境变量数组中找
	if (!ep->name && !bp)
	  bp = ep;
	else if (ep->name && striequ (name, ep->name))  //有相同的就退出循环
	  break;
    }
    
    if (ep < &envvar[NVAR]) {   //有相同的，是break出的循环
	/* must have got a match, free old value */
	if (ep->value) {
	    free (ep->value); ep->value = 0;  //释放原来的空间
	}
    } else if (bp) { //没有相同的，存在有envvar数组中第一个空白的位置
	/* new entry */
	ep = bp;
	if (!(ep->name = malloc (strlen (name) + 1)))  //分配空间
	  return 0;
	strcpy (ep->name, name);  //数据拷贝到空间中
    } else {  //envvar数组中没有找到空白的位置，已经填满了
	return 0;
    }

    if (value) { //value存在
	if (!(ep->value = malloc (strlen (value) + 1))) {  //分配空间
	    free (ep->name); ep->name = 0;
	    return 0;
	}
	strcpy (ep->value, value);  
    }

    return 1;
}

int
printvar(name,value, cntp)
	char *name;
	char *value;
	int  *cntp;
{
	const struct stdenv *ep;
	char buf[300];
	char *b;
#if (defined LOONGSON_3A2H) || (defined LOONGSON_3C2H)
	if(strstr(name,"syn0.ipaddr")!=NULL)
	{
                strcpy(name,"eth0.ipaddr");
	}
	if(strstr(name,"syn1.ipaddr")!=NULL)
        {
                strcpy(name,"eth1.ipaddr");
        }
	if(!value){
		sprintf(buf, "%10s", name);
	}
	else if(strchr(value, ' ')) {
		sprintf(buf, "%10s = \"%s\"", name, value);
	}
	else {
		sprintf(buf, "%10s = %-10s", name, value);
	}
	if(strstr(name,"eth0.ipaddr")!=NULL)
	{
                strcpy(name,"syn0.ipaddr");
	}
	if(strstr(name,"eth1.ipaddr")!=NULL)
        {
                strcpy(name,"syn1.ipaddr");
        }
#else 
#if defined (LOONGSON_3ASINGLE) || defined (LOONGSON_3BSINGLE)
	if(strstr(name,"rte0.ipaddr")!=NULL)
        {
                strcpy(name,"eth0.ipaddr");
        }
	if(!value){
		sprintf(buf, "%10s", name);
	}
	else if(strchr(value, ' ')) {
		sprintf(buf, "%10s = \"%s\"", name, value);
	}
	else {
		sprintf(buf, "%10s = %-10s", name, value);
	}
	if(strstr(name,"eth0.ipaddr")!=NULL)
        {
                strcpy(name,"rte0.ipaddr");
        }
#else
#if defined (LOONGSON_3ASERVER) || defined (LOONGSON_3BSERVER)
	if(strstr(name,"em0.ipaddr")!=NULL)
        {
		strcpy(name,"eth0.ipaddr");
	}	
	if(strstr(name,"em1.ipaddr")!=NULL)
        {
		strcpy(name,"eth1.ipaddr");
	}		
	if(!value){
		sprintf(buf, "%10s", name);
	}
	else if(strchr(value, ' ')) {
		sprintf(buf, "%10s = \"%s\"", name, value);
	}
	else {
		sprintf(buf, "%10s = %-10s", name, value);
	}	
	if(strstr(name,"eth0.ipaddr")!=NULL)
        {
		strcpy(name,"em0.ipaddr");
	}	
	if(strstr(name,"eth1.ipaddr")!=NULL)
        {
		strcpy(name,"em1.ipaddr");
	}	
#else
	if(!value){
		sprintf(buf, "%10s", name);
	}
	else if(strchr(value, ' ')) {
		sprintf(buf, "%10s = \"%s\"", name, value);
	}
	else {
		sprintf(buf, "%10s = %-10s", name, value);
	}	
#endif
#endif
#endif
	b = buf + strlen(buf);
	if ((ep = getstdenv(name)) && ep->values) {
		sprintf (b, "  [%s]", ep->values);
	}
	return(more(buf, cntp, moresz));
}

int
setenv (name, value)
    char *name, *value;
{
	return(do_setenv (name, value, 0));
}

int
do_setenv (name, value, temp)
	char *name, *value;
	int temp;
{
	if (_setenv (name, value)) {
#ifdef HAVE_NVENV
		const struct stdenv *sp;
		if ((sp = getstdenv (name)) && striequ (value, sp->init)) {
			/* set to default: remove from non-volatile ram */
			return tgt_unsetenv (name);
		}
		else if(!temp) {
			/* new value: save in non-volatile ram */
			return tgt_setenv (name, value);
		}
		else {
			return(1);
		}
#endif
	}
	return 0;
}

char *
getenv(name)
    const char *name;
{

    if (envinited) {
	struct envpair *ep;
	for (ep = envvar; ep < &envvar[NVAR]; ep++)
	  if (ep->name && striequ (name, ep->name))
	    return ep->value ? ep->value : "";
    } else {
	const struct stdenv *sp;
	if ((sp = getstdenv (name)) != 0)
	    return sp->init;
    }
    return 0;
}


void
mapenv (pfunc)
    void (*pfunc) __P((char *, char *));
{
    struct envpair *ep;

    for (ep = envvar; ep < &envvar[NVAR]; ep++) {
	if (ep->name)
	  (*pfunc) (ep->name, ep->value);
    }
}
 
void
unsetenv (pat)
    const char *pat;
{
	const struct stdenv *sp;
	struct envpair *ep;
	int ndone = 0;

	for (ep = envvar; ep < &envvar[NVAR]; ep++) {
		if (ep->name && strpat (ep->name, pat)) {
			sp = getstdenv (ep->name);
			if (sp) {
				/* unsetenving a standard variable resets it to initial value! */
				if (!setenv (sp->name, sp->init))
					return;
			} else {
				/* normal user variable: delete it */
#ifdef HAVE_NVENV				
				tgt_unsetenv (ep->name);
#endif
				free (ep->name);
				if (ep->value)
					free (ep->value);
				ep->name = ep->value = 0;
			}
			ndone++;
		}
	}
}

void
envinit ()
{
	int i;

    SBD_DISPLAY ("MAPV", CHKPNT_MAPV);   //打印提示

    /* extract nvram variables into local copy */
    bzero (envvar, sizeof(envvar));   //清零缓存
    tgt_mapenv (_setenv);           //从flash中读出环境变量，注意参数是个函数指针
    envinited = 1;

    SBD_DISPLAY ("STDV", CHKPNT_STDV);    //打印提示
    /* set defaults (only if not set at all) */
    for (i = 0; stdenvtab[i].name; i++) {    //这里是默认的标准环境变量的读取
	if (!getenv (stdenvtab[i].name)) {
	  setenv (stdenvtab[i].name, stdenvtab[i].init);
	}
    }
}

/** matchenv(name) returns integer corresponding to current value */
int
matchenv (name)
     char           *name;
{
    const struct stdenv *ep;
    char           *value;

    if (!(ep = getstdenv (name)))
	return (-1);

    if (!(value = getenv (name)))
      value = "";

    return _matchval (ep, value);
}

/* Two helpers to build a traditional environment for clients */

#ifndef BOOT_PARAM
void
envsize (int *ec, int *slen)
{
	struct envpair *ep;

	*ec = 0;
	*slen = 0;
	for (ep = envvar; ep < &envvar[NVAR]; ep++) {
		if (ep->name) {
			*ec += 1;
			*slen += strlen (ep->name) + 2; /* = and NULL */
			if (ep->value) {
				*slen += strlen(ep->value);
			}
		}
	}
}

void
envbuild (char **vsp, char *ssp)
{
	struct envpair *ep;

	for(ep = envvar; ep < &envvar[NVAR]; ep++) {
		if (ep->name) {
			*vsp++ = ssp;
			ssp += sprintf (ssp, "%s=", ep->name) + 1;
			if (ep->value) {
				ssp += sprintf(ssp - 1, "%s", ep->value);
			}
		}
	}
	*vsp++ = (char *)0;
}
#else
void envsize(int *ec, int* slen)
{
  *ec =1; 
  *slen = sizeof(struct boot_params);
	printf ("%s:length of boot_param is %08x\n", __FILE__, *slen);
}

void envbuild(char **vsp, char *ssp)
{

	struct boot_params * bp;
	bp = (struct boot_params *) ssp;

  init_boot_param(bp);
}
#endif
