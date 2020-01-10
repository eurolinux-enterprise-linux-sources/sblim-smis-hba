/*
 * $Id: Linux_Common.c,v 1.1.1.1 2009/05/12 21:46:33 nsharoff Exp $
 *
 * (C) Copyright IBM Corp. 2002, 2009
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 *
 * You can obtain a current copy of the Eclipse Public License from
 * http://www.opensource.org/licenses/eclipse-1.0.php
 *
 * Author:       Heidi Neumann <heidineu@de.ibm.com>
 * Contributors: Viktor Mihajlovski <mihajlov@de.ibm.com>
 *               C. Eric Wu <cwu@us.ibm.com>
 *
 * Contributors: IBM
 *
 * Description:
 * This shared library provides common functionality for OS Base 
 * Instrumentation, It is independent from any specific CIM class or CIM 
 * technology at all.
 *
*/

#include "Linux_Common.h"

#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>  
#include <dirent.h>
#include <sys/stat.h>    
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
/*#include <sys/cfgodm.h>*/
#include <nl_types.h>
#include <utmp.h>
#include <time.h>

#define     BOOT_MSG        "system boot"
char * CIM_OS_NAME            = NULL;
int    CIM_OS_TIMEZONE        = 999;
time_t CIM_OS_BOOTTIME        = 0;

/* ---------------------------------------------------------------------------*/

extern int    _debug;
char * _LINUX_PROVIDER_TRACE_FILE = NULL;
static int raw_run_command(const char *cmd, char **in, char ***out, char ***err, int have_line);

/* initialization routine */
void initialize() {

  char * var  = NULL;
  char * err  = NULL;
  FILE * ferr = NULL;

  _OSBASE_TRACE(4,("--- initialize() called"));

  /* get tracing setting from LINUX_PROVIDER_TRACE_??? */
  _debug = 0;
  var = getenv("LINUX_PROVIDER_TRACE_LEVEL");
  if( var != NULL ) {
    _debug = atoi(var);
    err = getenv("LINUX_PROVIDER_TRACE_FILE");
  }
  else {
    /* get tracing setting from DIRECTOR_PROVIDER_TRACE_??? */
    var = getenv("DIRECTOR_PROVIDER_TRACE_LEVEL");
    if( var != NULL ) {
      _debug = atoi(var);
      err = getenv("DIRECTOR_PROVIDER_TRACE_FILE");
    }
    else {
      _debug = 0;
    }
  }

  if( err != NULL ) {
    if( ( ((ferr=fopen(err,"a")) == NULL) || fclose(ferr) ) ) {
      fprintf(stderr,"Couldn't create trace file\n");
      return;
    }
    _LINUX_PROVIDER_TRACE_FILE = strdup(err);
  }
  else {
    if(_LINUX_PROVIDER_TRACE_FILE) free(_LINUX_PROVIDER_TRACE_FILE);
    _LINUX_PROVIDER_TRACE_FILE = NULL ;
  }
}

/* deinitialization routine */
void finish() {
    _OSBASE_TRACE(4,("--- finish() called"));

        /* Commenting the following line since we do not allocate
           any space for CIM_OS_NAME. We just hard code it to Linux 
        free(CIM_OS_NAME);
        */

    if (_LINUX_PROVIDER_TRACE_FILE) free(_LINUX_PROVIDER_TRACE_FILE);
}

/* ---------------------------------------------------------------------------*/


/* initializes the variable hostname of system
 * contains the full qualified IP hostname of the system, e.g. host.domain
 */
/* LinuxPORT */
char * get_system_name() {
  char *  host   = NULL;
  char *  domain = NULL;
  char *  ptr    = NULL;
  char ** hdout  = NULL;
  char *  fullname = NULL;
  int     rc     = 0;

  _OSBASE_TRACE(4,("--- get_system_name() called : init"));

  host = (char*)calloc(1,255);
  if ( gethostname(host, 255 ) == -1 ) { return NULL ; }
  /* if host does not contain a '.' we can suppose, that the domain is not
   * available in the current value. but we try to get the full qualified
   * hostname.
   */
  if( strchr( host , '.') == NULL ) {
    /* get domain name */
    rc=runcommand("/bin/dnsdomainname",NULL,&hdout,NULL);

    /*resolv.conf may not exist so the returned value may be empty.*/ 
    /*rc=runcommand("/bin/cat /etc/resolv.conf 2>/dev/null |/usr/bin/grep domain|/usr/bin/awk '{print $2}'",NULL,&hdout,NULL);*/

    if (rc == 0 && hdout != NULL) {
      if (hdout[0]) {
        domain = strdup(hdout[0]);
        ptr = strchr(domain, '\n');
        *ptr = '\0';
      }        
    }       
    if(hdout) freeresultbuf(hdout);
  }

  /* construct host name */
  if( strlen(host) ) {
    if( !domain ) {
      fullname = (char*)calloc( 1, (strlen(host)+1));
      strcpy( fullname, host);
    }
    else {
      fullname = (char*)calloc(1, (strlen(host)+strlen(domain)+2));
      strcpy( fullname, host);
      strcat( fullname, ".");
      strcat( fullname, domain );
    }
  }
  if(host) free(host);
  if(domain) free(domain);

  _OSBASE_TRACE(4,("--- get_system_name() : fullname is initialized with %s",fullname));

  return fullname;
}


/* Hardcoding it to string "Linux" */
char * get_os_name(){


  _OSBASE_TRACE(4,("--- get_os_name() called : init"));
   
   if( !CIM_OS_NAME ) CIM_OS_NAME = "Linux";

  _OSBASE_TRACE(4,("--- get_os_name() : CIM_OS_NAME initialized with %s",CIM_OS_NAME));
  
  return CIM_OS_NAME;


}


/* LinuxPORT */
signed short get_os_timezone() {

  struct timezone tz;
  struct timeval tv;
  int tZone = 0;

  if(CIM_OS_TIMEZONE == 999) {

    _OSBASE_TRACE(4,("--- get_os_timezone() called : init"));

    if (gettimeofday(&tv, &tz) == 0) { // Success
    // Minus sign to indicate west. Will also work with other locals
      CIM_OS_TIMEZONE = -tz.tz_minuteswest;
      _OSBASE_TRACE(4,("--- get_os_timezone() called : CIM_OS_TIMEZONE initialized with %i",CIM_OS_TIMEZONE));
    }
    else {
      _OSBASE_TRACE(4,("--- get_os_timezone() failed : CIM_OS_TIMEZONE initialized with 0"));
      CIM_OS_TIMEZONE = 0;
    }
  }
  return CIM_OS_TIMEZONE;
}


/* Linux VERSION */
time_t _get_os_boottime() {
    struct utmp buf[128];
    char filename[PATH_MAX];
    struct tm cttm;
    struct utmp *bp;
    struct stat stb;
    int i=0,bl,wtmp;
    char* bt = NULL;

    _OSBASE_TRACE(4,("--- get_os_boottime() called"));
    if (CIM_OS_BOOTTIME == 0) {
    setutent();
    while ((bp = getutent()) != NULL) {
      if (bp->ut_type == BOOT_TIME) {
        if (strncmp(bp->ut_line, BOOT_MSG, sizeof(BOOT_MSG)-1) == 0) {
          CIM_OS_BOOTTIME = bp->ut_time;
          break;
        }
      }
    }
    endutent();
  }
  _OSBASE_TRACE(4,("--- get_os_boottime() exited : %i",CIM_OS_BOOTTIME));
  return CIM_OS_BOOTTIME;
}


/* add timezone to datetime 'str'of format %Y%m%d%H%M%S.000000
 * out : yyyyMMddHHmmss.SSSSSSsutc, e.g. 20030112152345.000000+100
 */
void _cat_timezone( char * str , signed short zone ) {
  char * tz = NULL;

  tz = (char*)calloc(1,5);
  sprintf(tz, "%+04d", zone);
  if( str != NULL ) strcat(str,tz);
  if(tz) free(tz);
}


/* ---------------------------------------------------------------------------*/
/* tool function : executes commandlines and returns their output             */
/* ( stdout and stderr )                                                      */
/* ---------------------------------------------------------------------------*/
static void addstring(char ***buf, const char *str);
static void out_runcommand_line(FILE* p, size_t BufSize, char* buffer, char ***out)
{
  char* buffer1 = NULL;
  char *tmp = NULL;
  int CatMore=0;
       
  if (strlen(buffer) == (BufSize-1) && buffer[BufSize-2] != '\n')
  {
    buffer1 = (char*)malloc(BufSize);
    strcpy(buffer1,buffer);
		
    while(fgets(buffer,BufSize,p) != NULL)
    {
      CatMore++;
      buffer1 = (char*)(realloc(buffer1, (CatMore+1)*BufSize));
      tmp = buffer1 + CatMore*(BufSize-1);
      strcpy(tmp, buffer);
      if ((strlen(buffer) == (BufSize-1) && buffer[BufSize-2] == '\n') || strlen(buffer) < (BufSize-1))
        break;
      }
      if (CatMore == 0)
        buffer1 = buffer;
    }     
    else
      buffer1 = buffer;

    addstring(out,buffer1);
    if (CatMore != 0)
      free(buffer1);
}

/*
 * function raw_run_command excuted the give command line and redirect the stdout and stderr 
 * to a buffer array.
 * If parameter have_line is 1, the each line of output would hold the '\n' at the line ending.
 * Otherwise the '\n' would be tripped.
 */
static int raw_run_command(const char *cmd, char **in, char ***out, char ***err, int have_line)
{
    int rc;
    char fcltinname[] = "/tmp/LinuxXXXXXX";
    char fcltoutname[] = "/tmp/LinuxXXXXXX";
    char fclterrname[] = "/tmp/LinuxXXXXXX";
    int fdin, fdout, fderr;
    char * cmdstring;
    char * buffer;
    FILE * fcltin;
    FILE * fcltout;
    FILE * fclterr;

    /* No pipes -- real files */

    /* make the filenames for I/O redirection */
    if (in != NULL && ((fdin=mkstemp(fcltinname)) < 0 || close(fdin))) {
        _OSBASE_TRACE(4,("%s --- Could'nt create input file", strerror(errno)));
        return -1;
    }
    if (out != NULL && ((fdout=mkstemp(fcltoutname)) < 0 || close(fdout))) {
        _OSBASE_TRACE(4,("%s --- Couldn't create output file", strerror(errno)));
        return -1;
    }
    if (err != NULL && ((fderr=mkstemp(fclterrname)) < 0 || close(fderr))) {
        _OSBASE_TRACE(4,("%s --- Couldn't create error file", strerror(errno)));
        return -1;
    }

    /* if necessary feed in-buffer to child's stdin */
    if (in != NULL) {
        fcltin = fopen(fcltinname,"w");
        if (fcltin==NULL) {
            _OSBASE_TRACE(4,("%s --- Couldn't open client's input for writing", strerror(errno)));
            return -1;
        }
        while (*in != NULL)
            fprintf(fcltin,"%s\n",*in++);
        /* flush and close */
        fclose(fcltin);
    }

    /* prepare full command string */
    cmdstring = (char*)malloc(strlen(cmd) + 1 +
            (in?strlen(fcltinname)+2:0) +
            (out?strlen(fcltoutname)+3:0) +
            (err?strlen(fclterrname)+3:0));
    strcpy(cmdstring,cmd);
    if (in) {
        strcat(cmdstring," <");
        strcat(cmdstring,fcltinname);
    }
    if (out) {
        strcat(cmdstring," >");
        strcat(cmdstring,fcltoutname);
    }
    if (err) {
        strcat(cmdstring," 2>");
        strcat(cmdstring,fclterrname);
    }

    /*printf("quliang: raw cmd is: %s\n", cmdstring);*/
    
    /* perform the system call */
    _OSBASE_TRACE(4,("--- runcommand() : %s",cmdstring)); 
    rc=system(cmdstring);
    free(cmdstring);

    buffer = NULL;
    /* cleanup in file. if necessary */
    if (in != NULL)
        unlink(fcltinname);
    /* retrieve data from stdout and stderr */
    if (out != NULL) {
        *out=(char**)calloc(1,sizeof(char*));
        buffer = (char*)malloc(4096);
        memset(buffer, 0, 4096); /*make c style string*/
        fcltout = fopen(fcltoutname,"r");
        if (fcltout!=NULL) {
            while(!feof(fcltout)) {
                if (fgets(buffer,4096,fcltout) != NULL) {
                    if (buffer[strlen(buffer) - 1] == '\n' && (have_line == 0))
                        buffer[strlen(buffer) - 1] = '\0';
                    out_runcommand_line(fcltout, 4096, buffer, out);
                }
                /* addstring(out,buffer);*/
                else
                    break;
            }
            /* close out file */
            fclose(fcltout);
        } else
        _OSBASE_TRACE(4,("%s --- Couldn't open client's output for reading", strerror(errno)));
        unlink(fcltoutname);
    }
    if (err != NULL) {
        *err=(char**)calloc(1,sizeof(char*));
        if (buffer == NULL) 
            buffer = (char*)malloc(4096);
        memset(buffer, 0, 4096); /*make c style string*/
        fclterr = fopen(fclterrname,"r");
        if (fclterr!=NULL) {
            while(!feof(fclterr)) {
                if (fgets(buffer,4096,fclterr) != NULL) {
                    if (buffer[strlen(buffer) - 1] == '\n' && (have_line == 0))
                        buffer[strlen(buffer) - 1] = '\0';
                    out_runcommand_line(fclterr, 4096, buffer, err);
                }
                /*addstring(err,buffer);*/
                else
                    break;
            }
            /* close err file */
            fclose(fclterr);
        } else
        _OSBASE_TRACE(4,("%s --- Couldn't open client's error output for reading", strerror(errno)));
        unlink(fclterrname);
    }
    if (buffer != NULL)
        free(buffer);
    return rc;
}

/* execute the given command line, return result in buffer array.
   Each line is without ending '\n'
 */
int exec_command(const char *cmd, char **in, char ***out, char ***err)
{
    return raw_run_command(cmd, in, out, err, 0);
}

/* execute the given command line, return result in buffer array.
   Each line is with an ending '\n'
 */
int runcommand(const char *cmd, char **in, char ***out, char ***err)
{
    return raw_run_command(cmd, in, out, err, 1);
}

void freeresultbuf(char ** buf)
{
  char ** bufstart = buf;
  if (buf == NULL)
    return;
  while(*buf != NULL) {
    free(*buf++);
  }
  free(bufstart);
}

static void addstring(char *** buf, const char * str)
{
  int i=0;
  while ((*buf)[i++] != NULL);
  *buf= (char**)realloc(*buf,(i+1)*sizeof(char*));
  (*buf)[i-1] = strdup(str);
  (*buf)[i] = NULL;
}

/* ---------------------------------------------------------------------------*/
// end of tool function to executes commandlines
/* ---------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------------*/
// tool function to convert ":"-seperated lines
/* ---------------------------------------------------------------------------*/

/* converts a line into an array of char * , while
 * c ( e.g. ':' ) acts as seperator
 *
 * example :
 * line  -> value1:value2:value3'\n'
 *
 * array -> value1
 *          value2
 *          value3
 *          NULL
 */

char ** line_to_array( char * buf , int c ){
  char ** data = NULL ;
  char *  str  = NULL ;
  char *  ptr  = NULL ;
  char *  ent  = NULL ;

  str = strdup(buf);
  data = (char**)calloc(1,sizeof(char*));
  ent = str ;

  while( ( ptr = strchr( ent , c )) != NULL ) {
    *ptr='\0';
    addstring( &data , ent );
    ent = ptr+1;
    ptr = NULL ;
  }

// Linux Port - added following lines to handle empty spaces at the end 
// for Unix Process enumeration calls for Parameters array

  if ((ptr = strchr( ent , c )) != NULL ) {
  	addstring( &data , ent );
	ptr=NULL;
  }
  else
	addstring( &data, '\0'  );


  if(str) free(str);
  return data;
}

/* ---------------------------------------------------------------------------*/
// end of tool function to convert ":"-seperated lines
/* ---------------------------------------------------------------------------*/



/* ---------------------------------------------------------------------------*/
/*
 * get system parameter from a file entry, such as /proc/sys/kernel/shmmax
 * and put it in the character array pointed by buffer
 * It returns the number of bytes read, or -1 in case of any error
 */

int get_system_parameter(char *path, char *entry, char *buffer, int size) {
  char * completePath = NULL;
  FILE * file         = NULL;
  int    res          = 0;

  _OSBASE_TRACE(4,("--- get_system_parameter() called"));

  if (path == NULL || entry == NULL || buffer == NULL)
    return -1;
  completePath = (char *)malloc(strlen(path) + strlen(entry) + 1);
  if (completePath == NULL)
    return -1;
  sprintf(completePath, "%s/%s", path, entry);
  if ((file = fopen(completePath, "r")) == NULL)
    return -1;
  res = fread(buffer, 1, size - 1, file);
  fclose(file);
  if (res > 0)
    buffer[res] = '\0';
  free(completePath);

  _OSBASE_TRACE(4,("--- get_system_parameter() exited"));
  return res;
}


/* ---------------------------------------------------------------------------*/
/*
 * set system parameter to a file entry, such as /proc/sys/kernel/shmmax
 * using the value string 
 * It returns the number of bytes written, or -1 in case of any error
 */

int set_system_parameter(char *path, char *entry, char *value) {
  char * completePath = NULL;
  FILE * file         = NULL;
  int    res          = 0;

  _OSBASE_TRACE(4,("--- set_system_parameter() called"));

  if (path == NULL || entry == NULL || value == NULL)
    return -1;
  completePath = (char *)malloc(strlen(path) + strlen(entry) + 1);
  if (completePath == NULL)
    return -1;
  sprintf(completePath, "%s/%s", path, entry);
  if ((file = fopen(completePath, "w")) == NULL)
    return -1;
  res = fwrite(value, 1, strlen(value), file);
  fclose(file);
  free(completePath);
  
  _OSBASE_TRACE(4,("--- set_system_parameter() exited"));
  return res;
}


/* ---------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/*                            trace facility                                  */
/* ---------------------------------------------------------------------------*/

char * _format_trace(char *fmt,...) {
   va_list ap;
   char *msg=(char*)malloc(1024);
   va_start(ap,fmt);
   vsnprintf(msg,1024,fmt,ap);
   va_end(ap);
   return msg;
}

void _osbase_trace( int level, char * file, int line, char * msg) {

  struct tm        cttm;
  struct timeval   tv;
  struct timezone  tz;
  /* LinuxPORT * - changed long to time_t */
  time_t  	   sec  = 0;
  char           * tm   = NULL;
  FILE           * ferr = NULL;

  if( (_LINUX_PROVIDER_TRACE_FILE != NULL) ) {
    if( (ferr=fopen(_LINUX_PROVIDER_TRACE_FILE,"a")) == NULL ) {
      fprintf(stderr,"Couldn't open trace file");
      return;
    }
  }
  else { ferr = stderr; }

  if( gettimeofday( &tv, &tz) == 0 ) {
    //sec = tv.tv_sec + (tz.tz_minuteswest*-1*60);
    // Do not minus the 360 (minuteswest) for the localtime
    sec = tv.tv_sec;
    tm = (char*)malloc(20);
    memset(tm, 0, 20*sizeof(char));
    if( localtime_r( &sec , &cttm) != NULL ) {
      strftime(tm,20,"%m/%d/%Y %H:%M:%S",&cttm);
    }
  }
  fprintf(ferr,"[%i] [%s %ld] --- %s(%i) : %s\n", level, tm, tv.tv_usec, file, line, msg);

  if( (_LINUX_PROVIDER_TRACE_FILE != NULL) ) {
    fclose(ferr);
  }

  if(tm) free(tm);
  if(msg)  free(msg);
}

/*
 * Get the device description of a device from the catalog files
 * the catalog set no. and message no. could be get form CuAt or PdAt
 * (attribute name is 'message_no). If the attribute cannot be found, 
 * the msg_no of PdDv be used.
 * parameter: devname. the device name
 * return value: an allocated buffer that hold the description of the
 *      device. return NULL if no description could be found.
 * It's caller that in charge of the free of returned buffer.
 */
char * get_device_desc(const char* devname) {
    return NULL;
}

