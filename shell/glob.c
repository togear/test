/* 
 * glob.c��UNIX V6�µ�ͨ�����չ����(utility)����װ��/etc/glob��Դ��λ��http://minnie.tuhs.org/Archive/PDP-11/Distributions/research/Dennis_v6/ 
 * UNIX v6����BSD���֤�������������������glob.c��ԭ������Ken Thompson(1943 - )�� 
 * ������Ҫ�����Ƕ�Դ���������ע�⣬����K&R Cת��ANSI C������֮��δ���κθĶ��� 
 * ���ļ���gcc-4.4.3��ͨ�����룬�����˲��ܵ�����shellӦ�����κν���������κ����⡣
 * �κ��˿�����ѧϰ����д�������·�����������;������ѭBSD���Э�顣 
 * ����޶�ʱ��:  2013-4-19 
 * �޶���:  Leo Ma 
 * ��ϵ��ʽ: begeekmyfriend@gmail.com 
 */ 
 
/* global command --

   glob params

   "*" in params matches r.e ".*"
   "?" in params matches r.e. "."
   "[...]" in params matches character class
   "[...a-z...]" in params matches a through z.

   perform command with argument list
  constructed as follows:
     if param does not contain "*", "[", or "?", use it as is
     if it does, find all files in current directory
     which match the param, sort them, and use them

   prepend the command name with "/bin" or "/usr/bin"
   as required.
*/

#include <stdlib.h>

#define	E2BIG	7
#define	ENOEXEC	8
#define	ENOENT	2

#define	STRSIZ	522
char	ab[STRSIZ];		/* ͨ�����չ���ַ����� */
char	*ava[200];		/* �����б��� */
char	**av;		/* �����б� */
char	*string;	/* �����滻ƥ����ļ��� */
int	errno;
int	ncoll;

int compar(char *as1, char *as2);
char *cat(char *as1, char *as2);
void sort(char **oav);
void expand(char *as);
void toolong();
void execute(char *afile, char **aarg);
int umatch(char *s, char *p);
int amatch(char *as, char *ap);
int match(char *s, char *p);

void main(int argc, char *argv[])
{
	char *cp;

	string = ab;
	av = &ava[1];

	if (argc < 3) {
		write(2, "Arg count\n", 10);
		return;
	}
	argv++;
	*av++ = *argv;
	while (--argc >= 2)
		expand(*++argv);	/* ��չͨ��� */
	if (ncoll==0) {
		write(2, "No match\n", 9);	/* ͨ�����ƥ�� */
		return;
	}
	execute(ava[1], &ava[1]);	/* ִ�б��ص�·�� */
	cp = cat("/usr/bin/", ava[1]);
	execute(cp+4, &ava[1]);		/* ִ��/bin�µ�·�� */
	execute(cp, &ava[1]);	/* ִ��/usr/bin�µ�·�� */
	write(2, "Command not found.\n", 19);	/* ʧ�� */
}

void expand(char *as)
{
	char *s, *cs;
	int dirf;
	char **oav;
	static struct {
		int	ino;
		char	name[16];
	} entry;

	s = cs = as;
	while (*cs!='*' && *cs!='?' && *cs!='[') {
		if (*cs++ == 0) {
			*av++ = cat(s, "");	/* �Ҳ���ͨ�������������б�ֱ�ӷ��� */
			return;
		}
	}
	for (;;) {	/* ����ͨ��� */
		if (cs==s) {	/* ���ͨ���֮ǰû��'/'�ַ� */
			dirf = open(".", 0);	/* �򿪵�ǰĿ¼ */
			s = "";
			break;
		}
		if (*--cs == '/') {	/* ����ͨ���ǰ�������'/' */
			*cs = 0;	/* ��Ŀ¼·�����ļ����иsָ��Ŀ¼·����csָ���ļ��� */
			dirf = open(s==cs? "/": s, 0);	/* �����һ��Ŀ¼ */
			*cs++ = 0200;	/* ���ñ�ǣ��ں����cat()�лָ�Ϊ'/' */
			break;
		}
	}
	if (dirf<0) {
		write(2, "No directory\n", 13);		/* Ŀ¼������ */
		exit(-1);
	}
	oav = av;
	while (read(dirf, &entry, 16) == 16) {	/* ÿ�ζ�ȡһ���ļ�16���ֽڵ�Ԫ���� */
		if (entry.ino==0)
			continue;
		if (match(entry.name, cs)) {	/* nameΪʵ���ļ�����csΪ����ͨ�����ģʽƥ�� */
			*av++ = cat(s, entry.name);	/* ��ƥ����ļ����滻�������б��� */
			ncoll++;
		}
	}
	close(dirf);
	sort(oav);	/* �������������� */
}

/* Bubble sorting��ע�����Ƕ��ַ����б����� */
void sort(char **oav)
{
	char **p1, **p2, **c;

	p1 = oav;
	while (p1 < av-1) {
		p2 = p1;
		while(++p2 < av) {
			if (compar(*p1, *p2) > 0) {
				*c = *p1;
				*p1 = *p2;
				*p2 = *c;
			}
		}
		p1++;
	}
}

void execute(char *afile, char **aarg)
{
	char *file, **arg;

	arg = aarg;
	file = afile;
	execv(file, arg);
	if (errno==ENOEXEC) {	/* ����Ҳ����������ִ��sh */
		arg[0] = file;
		*--arg = "/bin/sh";
		execv(*arg, arg);
	}
	if (errno==E2BIG)
		toolong();
}

/* �ļ���̫�� */
void toolong()
{
	write(2, "Arg list too long\n", 18);
	exit(-1);
}

/*
 * s: string
 * p: pattern
 */
int match(char *s, char *p)
{
	if (*s=='.' && *p!='.')	/* �����ļ��Ĵ��� */
		return(0);
	return(amatch(s, p));
}

/* �����switch-case�÷��ȽϹ��죬����˵ݹ���ã�ע����ϸ��Ŷ~ */
int amatch(char *as, char *ap)
{
	char *s, *p;
	int scc;
	int c, cc, ok, lc;

	s = as;
	p = ap;
	if (scc = *s++)
		if ((scc &= 0177) == 0)
			scc = 0200;	/* ֮ǰ���������λ�ǿգ�����Ϊ�����ַ� */
	switch (c = *p++) {

	case '[':
		ok = 0;
		lc = 077777;	/* �˽��ƣ���ʼ��2^15Ϊ���ֵ */
		while (cc = *p++) {	/* ��ȡ��һ���ַ� */
			if (cc==']') {
				if (ok)
					return(amatch(s, p));	/* recursive */
				else
					return(0);	/* û��һ���ַ�ƥ�� */
			} else if (cc=='-') {
				if (lc<=scc && scc<=(c = *p++))	/* scc��ǰһ���ַ�lc�ͺ�һ���ַ�c��������Χ֮�� */
					ok++;
			} else
				if (scc == (lc=cc))	/* ��ֵ��lc��ƥ��"[]"������һ���ַ� */
					ok++;
		}
		return(0);
/* �������ͨ��������Ҳ���'\0'���ͻ���ת��default��ֵ��ע���������û��һ��break */
	default:
		if (c!=scc)
			return(0);

	case '?':	/* '?'ͨ�����ת������ */
		if (scc)
			return(amatch(s, p));	/* recursive */
		return(0);

	case '*':	/* '*'ͨ�����ת������ */
		return(umatch(--s, p));	/* ��Ҫ����һ���ַ� */

	case '\0':	/* pattern������ */
		return(!scc);	/* ���sccҲ��'\0'������1��������������ַ�������0 */
	}
}

int umatch(char *s, char *p)
{
	if(*p==0)
		return(1);	/* ��'*'����ģʽĩβ����ƥ�� */
	while(*s)
		if (amatch(s++,p))	/* �ݹ���ã�ֱ��ƥ��'*'��һ���ַ�ģʽ */
			return(1);
	return(0);
}

/* strcmp() */
int compar(char *as1, char *as2)
{
	char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while (*s1++ ==  *s2)
		if (*s2++ == 0)
			return(0);
	return (*--s1 - *s2);
}

/* Catenate as2 with as1 */
char *cat(char *as1, char *as2)
{
	char *s1, *s2;
	int c;

	s2 = string;
	s1 = as1;
	while (c = *s1++) {
		if (s2 > &ab[STRSIZ])	/* overflow! */
			toolong();
		c &= 0177;
		if (c==0) {
			*s2++ = '/';	/* Ŀ¼·��ĩβ����֮ǰexpand()�����ñ�ǻָ�Ϊ'/' */
			break;
		}
		*s2++ = c;	/* First put as1 into string */
	}
	s1 = as2;
	do {
		if (s2 > &ab[STRSIZ])	/* overflow! */
			toolong();
		*s2++ = c = *s1++;	/* and then put as2 into string */
	} while (c);
	s1 = string;	/* ָ��stringͷ�� */
	string = s2;	/* stringָ����һ���ַ� */
	return(s1);		/* ����ԭas1�����ַ���ַ */
}
