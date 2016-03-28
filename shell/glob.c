/* 
 * glob.c是UNIX V6下的通配符扩展工具(utility)，安装在/etc/glob，源码位于http://minnie.tuhs.org/Archive/PDP-11/Distributions/research/Dennis_v6/ 
 * UNIX v6是受BSD许可证保护的自由软件，其中glob.c的原作者是Ken Thompson(1943 - )。 
 * 本人主要工作是对源码进行重新注解，并将K&R C转成ANSI C，除此之外未做任何改动。 
 * 该文件在gcc-4.4.3下通过编译，但本人不能担保该shell应用于任何交互引起的任何问题。
 * 任何人可用作学习、改写或者重新发布等其它用途，请遵循BSD许可协议。 
 * 最近修订时间:  2013-4-19 
 * 修订人:  Leo Ma 
 * 联系方式: begeekmyfriend@gmail.com 
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
char	ab[STRSIZ];		/* 通配符扩展后字符缓存 */
char	*ava[200];		/* 参数列表缓存 */
char	**av;		/* 参数列表 */
char	*string;	/* 保存替换匹配的文件名 */
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
		expand(*++argv);	/* 扩展通配符 */
	if (ncoll==0) {
		write(2, "No match\n", 9);	/* 通配符不匹配 */
		return;
	}
	execute(ava[1], &ava[1]);	/* 执行本地的路径 */
	cp = cat("/usr/bin/", ava[1]);
	execute(cp+4, &ava[1]);		/* 执行/bin下的路径 */
	execute(cp, &ava[1]);	/* 执行/usr/bin下的路径 */
	write(2, "Command not found.\n", 19);	/* 失败 */
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
			*av++ = cat(s, "");	/* 找不到通配符，放入参数列表直接返回 */
			return;
		}
	}
	for (;;) {	/* 包含通配符 */
		if (cs==s) {	/* 如果通配符之前没有'/'字符 */
			dirf = open(".", 0);	/* 打开当前目录 */
			s = "";
			break;
		}
		if (*--cs == '/') {	/* 搜索通配符前面最近的'/' */
			*cs = 0;	/* 将目录路径与文件名切割，s指向目录路径，cs指向文件名 */
			dirf = open(s==cs? "/": s, 0);	/* 打开最低一级目录 */
			*cs++ = 0200;	/* 引用标记，在后面的cat()中恢复为'/' */
			break;
		}
	}
	if (dirf<0) {
		write(2, "No directory\n", 13);		/* 目录不存在 */
		exit(-1);
	}
	oav = av;
	while (read(dirf, &entry, 16) == 16) {	/* 每次读取一个文件16个字节的元数据 */
		if (entry.ino==0)
			continue;
		if (match(entry.name, cs)) {	/* name为实际文件名，cs为含有通配符的模式匹配 */
			*av++ = cat(s, entry.name);	/* 将匹配的文件名替换到参数列表中 */
			ncoll++;
		}
	}
	close(dirf);
	sort(oav);	/* 将参数排序后输出 */
}

/* Bubble sorting，注意这是对字符串列表排序 */
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
	if (errno==ENOEXEC) {	/* 如果找不到命令，重新执行sh */
		arg[0] = file;
		*--arg = "/bin/sh";
		execv(*arg, arg);
	}
	if (errno==E2BIG)
		toolong();
}

/* 文件名太长 */
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
	if (*s=='.' && *p!='.')	/* 隐藏文件的处理 */
		return(0);
	return(amatch(s, p));
}

/* 这里的switch-case用法比较诡异，结合了递归调用，注意仔细看哦~ */
int amatch(char *as, char *ap)
{
	char *s, *p;
	int scc;
	int c, cc, ok, lc;

	s = as;
	p = ap;
	if (scc = *s++)
		if ((scc &= 0177) == 0)
			scc = 0200;	/* 之前清除了引用位是空，则本身为引用字符 */
	switch (c = *p++) {

	case '[':
		ok = 0;
		lc = 077777;	/* 八进制，初始化2^15为最大值 */
		while (cc = *p++) {	/* 读取下一个字符 */
			if (cc==']') {
				if (ok)
					return(amatch(s, p));	/* recursive */
				else
					return(0);	/* 没有一个字符匹配 */
			} else if (cc=='-') {
				if (lc<=scc && scc<=(c = *p++))	/* scc在前一个字符lc和后一个字符c的连续范围之内 */
					ok++;
			} else
				if (scc == (lc=cc))	/* 赋值给lc，匹配"[]"内任意一个字符 */
					ok++;
		}
		return(0);
/* 如果不是通配符，并且不是'\0'，就会跳转到default，值得注意的是下面没有一处break */
	default:
		if (c!=scc)
			return(0);

	case '?':	/* '?'通配符跳转到这里 */
		if (scc)
			return(amatch(s, p));	/* recursive */
		return(0);

	case '*':	/* '*'通配符跳转到这里 */
		return(umatch(--s, p));	/* 需要回退一个字符 */

	case '\0':	/* pattern结束了 */
		return(!scc);	/* 如果scc也是'\0'，返回1；如果还有其它字符，返回0 */
	}
}

int umatch(char *s, char *p)
{
	if(*p==0)
		return(1);	/* 若'*'处于模式末尾，则匹配 */
	while(*s)
		if (amatch(s++,p))	/* 递归调用，直到匹配'*'下一个字符模式 */
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
			*s2++ = '/';	/* 目录路径末尾，将之前expand()中引用标记恢复为'/' */
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
	s1 = string;	/* 指向string头部 */
	string = s2;	/* string指向下一个字符 */
	return(s1);		/* 返回原as1处的字符地址 */
}
