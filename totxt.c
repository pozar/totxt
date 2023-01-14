#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef MSDOS
#include <conio.h>
char compdate[] = __DATE__;
char comptime[] = __TIME__;
#endif

#ifndef MAIN_H
#define MAIN_H
int banner(void);
int ws_help(void);
int eject(void);
int pageoff(void);
int totab(void);
int progname(void);
int printline(char *);
int filecopy(FILE *);
#endif

static char rcsid[] = "$Id: totxt.c,v 4.10 2023/01/14 22:29:34 pozar Exp $";

/*
 * TOTXT 
 *
 *  Changes text files to the local UNIX, MAC, or MS-DOS/CPM newline 
 *  conventions.
 *
 */

/*
 * REVISION HISTORY
 *
 *   4.8  Sat Jan 14 14:14:13 PST 2023
 *        Cleaned up to support C99 and the Xcode development env for the Mac.
 *
 *   4.1  Tue Aug  1 20:06:20 PDT 1995
 *        Put in -n switch to force a newline so the line is no longer than 'x'.
 *
 *   4.0  4.Apr.89
 *        Added support for a number of WordStar dot-commands.
 *        See '-wl' for supported commands.
 *
 *   3.3  23.Mar.89
 *        Added ROT180.  Which means, rotate between '!' and '~'.
 *        (proposed FidoNet 'standard')
 *
 *   3.2  5.Dec.88
 *        Added ROT13.  Which means, rotate between 'A' and 'Z', and 
 *        'a' and 'z'. (USENET 'standard')
 *
 *   3.1  9.Dec.87
 *        Added code to handle form-feeds (^L) properly.
 * 
 *   3.0  29.Nov.87
 *        Added -p switch for printer margin handling.
 *
 *   2.0  21.Oct.87 
 *        Added switches for tab handling and tab handling routines.
 *
 */

int ver = 4;    /* Current version and revision numbers. */
int rev = 1;
char date[] = "Tue Aug  1 20:06:20 PDT 1995\0";

#define FALSE 0
#define TRUE 1
#define NUL 0

#define WIDTH 80        /* output width */
#define TABSPACE 8      /* tab positions */

/* Globals ... */
int pagenumber = 1;	/* pagenumber holder */
int hpos = 1;           /* column position */
long forcedhpos = 0;     /* forced newline column position */
int vpos = 1;           /* line number */
int pl = 66;            /* total page length */
int mt = 3;             /* top margin */
int mb = 8;             /* bottom margin */
int po = 8;             /* page offset */
char header[80];	/* buffer for header string */
char footer[80];	/* buffer for footer string */
int rot13 = FALSE;	/* Rotate the characters 13 places for encryption
			   or decryption. (USENET 'standard') */
int rot180 = FALSE;	/* Rotate the characters between '!' and '~' for 
                           encryption or decryption. (proposed FidoNet) */
int hb_flag = FALSE;    /* flag to indicate if we are striping the high bit */
int tabstrip = FALSE;   /* flag to indicate if we are striping the tabs */
int printer = FALSE;    /* printer mode */
int ws_flag = FALSE;	/* reconize WordStar dot commands */
int forcednl_flag = FALSE;	/* forced newlines (see -n) */
int html_flag = FALSE;       /* flag to indicate if we are HTMLizing the text */
int bold_flag = FALSE;       /* flag to indicate if we are bolding in the HTML text */
char boldword[80];	/* Holds word for bolding */


int main(argc,argv)
int argc;
char *argv[];
{
FILE *fp, *fopen();
int i;

  if(argc == 1){
    banner();
    exit(0);
  } 

   /* scan command line arguments, and look for files to work on. */
   for (i = 1; i < argc; i++) {
      if (argv[i][0] != '-') {
        if((fp = fopen(argv[i],"rb")) == NULL) {
          printf("\rTOTXT: can't open %s  \n",argv[i]);
          break;
        } else {
          filecopy(fp);
          fclose(fp);
        }
      } else {
        switch (argv[i][1]){
          case 'H':	/* Spit out HTMLized text. */
          case 'h':
            html_flag = TRUE;
            break;
          case 'B':	/* What word to bold in the HTMLizing of the text */
          case 'b':
            if (isascii(argv[i+1][0])) {
              bold_flag = TRUE;
              i++;
              strcpy(boldword,argv[i]);
            }else{
              printf("Error: No argument for -B switch. (ie -B Bold)\n");
	      exit(1);
            }
            break;
          case 'R':
	  case 'r':
            if (argv[i][3] == '3')
	      rot13 = TRUE;
            if (argv[i][3] == '8')
	      rot180 = TRUE;
	    break;
          case '8':
            hb_flag = TRUE;
            break;
          case 'T':
          case 't':
            tabstrip = TRUE;
            break;
          case 'N':
          case 'n':
            if (isdigit(argv[i+1][0])) {
              forcednl_flag = TRUE;
              i++;
              forcedhpos = atol(argv[i]);
            }else{
              printf("So like, how many chars you want me to put a newline at?\n");
	      exit(1);
            }
            break;
          case 'W':
          case 'w':
            if ((argv[i][2] == 'l')||(argv[i][2] == 'L'))
              ws_help();
            tabstrip = TRUE;
            printer = TRUE;
            ws_flag = TRUE;
            break;
          case 'P':
          case 'p':
            tabstrip = TRUE;
            printer = TRUE;
            break;
          default:
             printf("I don't know the meaning of -%c.\n",argv[i][1]);
            banner();
            exit(1);
        }
      }
   }
}

int banner()
{
  progname();
  
printf("\
USAGE:\n\
TOTXT [switches] filename.ext [filename.ext filename.ext ...]\n\
\n\
  Changes UNIX, MAC, IBM-PC, or CP/M newlines in their text files into\n\
  the local enviorment's newline convention.  Also strips the high bit\n\
  off of the characters for word processed files, such as the output\n\
  files from WordStar(TM) MicroPro.\n\
\n\
  Output is via the standard output device.\n\
\n\
  Non-printing control characters will show as '^c'.  Where c is the\n\
  control character's name in upper-case (eg. ^G = bell).  ^@ and ^Z are\n\
  not displayed, but are tossed.\n\
\n\
  Currently does not support wild cards.\n\n");
#ifdef MSDOS
more();
#endif
printf("\
  SWITCHES:\n\
     -p    = Printer mode:\n\
               Strip tabs and replace with appropriate number of spaces.\n\
               Inserts a top, bottom, and left hand margin.\n\
     -8    = strip the 8th or high bit.\n\
     -t    = strip tabs and replace with appropriate number of spaces.\n\
     -n x  = Force a newline every 'x' characters.\n\
     -r13  = ROT13 encryption or decryption. (USENET 'standard')\n\
     -r180 = ROT180 encryption or decryption. (FidoNet proposed)\n\
     -w[l] = Support WordStar(TM) dot-commands.  Superset of '-p' flag.\n\
             If you enter a 'l' after the 'w', you will get the list of\n\
             dot-commands supported.\n\
\n\
  Files will be processed in the order of the command line.  If an command\n\
  line switch is put after a file name, the file will not be processed\n\
  with the action specified by the switch.\n\
  eg.  totxt FOO -t BOZO\n\
\n\
  The file named FOO will not have its tabs stripped out, but the\n\
  file BOZO will.\n\
");
}

int more()
{
char c;
   printf(" [more? Y,n] ");
#ifdef MSDOS
   while (!kbhit()){}
   c = getch();
#else
   c = getchar();
#endif
   if ((c == 'n')||(c =='N'))
     exit(0);
   printf("\r            \r");
}

int filecopy(fp)
FILE *fp;
{
char c;                 /* char to test and output */
char buff[80];		/* All around buffer */
int i,n;                /* All around variable */
int cr_flag = FALSE;    /* 'NEWLINE' flags ... */
int lf_flag = FALSE;

  if(html_flag)
    printf("<PRE>\n");

  c = getc(fp); 
  while (!feof(fp)){

	/* I know all these 'if' conditionals look ugly, but a case 
           switch here just wouldn't have worked... */
    while(!feof(fp)){
      if ((c == '.') && (hpos == 1) && ws_flag){	/* the dot must be the first 
						   	   thing on the line and the 
						   	   printer flag must be set.*/
        i = 0;
        while ((c = getc(fp)) != EOF){
          c = c & 0x7F;
          if (c == 0x12){
            c = '\'';
          }
          if (c == 10){	/* Line feed */
            lf_flag = TRUE;
            break;
          }
          if (c == 13){	/* Carriage return */
            cr_flag = TRUE;
            break;
          }
          buff[i++] = c;
          buff[i] = NUL;
        }
        if (!strncasecmp(buff,"he",2)){
          strcpy(header,buff+2);
        }
        if (!strncasecmp(buff,"fo",2)){
          strcpy(footer,buff+2);
        }
        if (!strncasecmp(buff,"mb",2)){
          if(strlen(buff) == 4){
            mb = (buff[2] - 0x30) * 10;
            mb = buff[3] - 0x30;
          } else
            mb = buff[2] - 0x30;
        }
        if (!strncasecmp(buff,"mt",2)){
          if(strlen(buff) == 4){
            mt = (buff[2] - 0x30) * 10;
            mt = buff[3] - 0x30;
          } else
            mt = buff[2] - 0x30;
        }
        if (!strncasecmp(buff,"pa",2)){
          eject();
        }
        if (!strncasecmp(buff,"pl",2)){
          if(strlen(buff) == 4){
            pl = (buff[2] - 0x30) * 10;
            pl = buff[3] - 0x30;
          } else
            pl = buff[2] - 0x30;
        }
        if (!strncasecmp(buff,"pn",2)){
          if(strlen(buff) == 4){
            pagenumber = (buff[2] - 0x30) * 10;
            pagenumber = buff[3] - 0x30;
          } else
            pagenumber = buff[2] - 0x30;
        }
        if (!strncasecmp(buff,"po",2)){
          if(strlen(buff) == 4){
            po = (buff[2] - 0x30) * 10;
            po = buff[3] - 0x30;
          } else
            po = buff[2] - 0x30;
        }
        while ((c = getc(fp)) != EOF){	/* get to the begining of the line */
          c = c & 0x7F;
          if ((c != 10)&&(c != 13))	/* Carriage return */
            break;
          if ((c == 10)&&(lf_flag)){
            lf_flag = FALSE;
            break;
          }
          if ((c == 13)&&(cr_flag)){
            cr_flag = FALSE;
            break;
          }
        }
      } else {
        break;
      }
    }

    if ((vpos == pl - (mb - 1)) && (printer)){	/* Do footer and margin */
      for (i = 0; i < mb - 1; i++){ /* Do bottom margin */
        printf("\n");
      }
      pageoff();
      printline(footer);	/* footer will be empty if -w not set */
      pagenumber++;
      vpos = 1;         /* At top of page again */
      hpos = 1;
    }

    if ((vpos == 1) && (printer)){	/* Do header and margin */
      pageoff();
      printline(header);	/* header will be empty if -w not set */
      for (i = 2; i <= mt; ++i){ /* Create top margin */
        printf("\n");
      }
      vpos = mt + 1;    /* At start of text vertical position */
      hpos = 1;
    }

    if (c == 10){	/* Line feed test */
      if (!cr_flag){
        printf("\n");
        lf_flag = TRUE;
        hpos = 1;
        vpos++;
      } else {
        cr_flag = FALSE;
      }
      c = getc(fp);
      c = c & 0x7F;
      continue;
    }

    if (!hb_flag){
      c = c & 0x7F;
    }

    if (c == 13){	/* Carriage return test */
      if (!lf_flag){
        printf("\n");
        cr_flag = TRUE;
        hpos = 1;
        vpos++;
      } else {
        lf_flag = FALSE;
      }
      c = getc(fp);
      c = c & 0x7F;
      continue;
    }

    if ((c == 0) || (c == 26)){	/* ignore ^@ and ^Z */
      c = getc(fp);
      c = c & 0x7F;
      continue; 
    }

    if ((hpos == 1) && (printer)){	/* do page offset (left margin */
      pageoff();
    }

    if (c == 9){	/* Tab test */
      n = totab();
      if (!tabstrip)    /* if not striping tabs then put out a real tab... */
        putchar(c);
      while (n > 0) {
        if (tabstrip)   /* ... and forget putting out spaces */
          putchar(' ');
        ++hpos;
        --n;
      }
      c = getc(fp);
      c = c & 0x7F;
      continue;
    }

    if (c == 12){       /* Form feed test */
      if (printer){     /* Eject the page */
        eject();
      } else {          /* Just stick it out there */
        putchar(c);
        cr_flag = FALSE;
        lf_flag = FALSE;
        ++hpos;
      }
      c = getc(fp);
      c = c & 0x7F;
      continue;
    }

    if (((c >= 0) && (c <= 0x8)) || ((c >= 0xD) && (c <= 0x1F))){
      putchar('^');
      putchar(c + 0x40);
      ++hpos;
    } else {
      if (rot13){
        if (((c >= 'A') && (c <= 'M')) || ((c >= 'a') && (c <= 'm'))){
          putchar(c + 0xd);
        }
        if (((c >= 'N') && (c <= 'Z')) || ((c >= 'n') && (c <= 'z'))){
          putchar(c - 0xd);
        }
        if (!(((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')))){
          putchar(c);
        }
      }
      if (rot180){
        if ((c >= '!')  && (c <= 'O')){
          putchar(c + 0x2f);
        }
        if ((c >= 'P')  && (c <= '~')){
          putchar(c - 0x2f);
        }
        if ((c < '!') || (c > '~')){
          putchar(c);
        }
      }
      if (!rot13 && !rot180){
        putchar(c);
      }
      cr_flag = FALSE;
      lf_flag = FALSE;
      ++hpos;

      if (forcednl_flag && (forcedhpos <= hpos)){
        putchar('\n');
        hpos = 1;
      }

    }
    c = getc(fp);
    c = c & 0x7F;
  }

  if(html_flag)
    printf("</PRE>\n");

  if (printer) {        /* Eject paper from printer */
    eject();
  }
}

int ws_help()
{
  progname();
  printf("\
  WordStar dot-commands supported:\n\
    .fo<text> = footer\n\
    .he<text> = header\n\
    .mb[8]    = bottom margin\n\
    .mt[3]    = top margin\n\
    .pa       = new page\n\
    .pl[66]   = page lenght\n\
    .pn[x]    = start page numbering with 'x', here\n\
    .po[8]    = page offset (left margin)\n\
    ..        = comment\n\
    Numbers in brackets([]), are default values.\n");
  exit(0);
}

int progname()
{
  printf("TOTXT  %s\n",rcsid);
  printf("Copyright 1987-2023 Timothy M. Pozar\n");
}

int pageoff()
{
int i;

  for (i = 0; i < po; i++){ /* Do page offset */
    printf(" ");
  }
  hpos = po + 1;    /* At start of text cursor position */
/*  printf("%d",vpos); */  /* Start of line number hack */
}

int totab()
{
int i;

  i = hpos - 1;
  while (i >= TABSPACE){
    i = i - TABSPACE;
  }
  return (TABSPACE - i);
}

/*
 * Ejects the page via newlines.
 *
 */
int eject() 
{
int i;

  for (i = vpos; i < pl; i++) {
    printf("\n");
  }
  if(printer)
    pageoff();
  printline(footer);		/* footer will be empty if -w not set */
  pagenumber++;
  vpos = 1;
  hpos = 1;
}

int printline(string)
char *string;
{
int i;

    for (i = 0; string[i] != 0; i++){
      if(string[i] == '#')
        printf("%d",pagenumber);
      else
        putchar(string[i]);
    }
    printf("\n");
}

/* 
 * #ifndef MSDOS
 * strnicmp()
 * {
 * }
 * #endif
 */
