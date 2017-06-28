

#include "ffs_func.h"
// #include "i2sdac.h"

//static const char tag[] = "[SPIFFS example]";
static const char tag[] = "[FFS_FUNC]";

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
// const int CONNECTED_BIT = 0x00000001;

static const int mOK_CODE=1;
static  const int mNG_CODE=0;


// ============================================================================
#include <ctype.h>

// fnmatch defines
#define	FNM_NOMATCH	1	// Match failed.
#define	FNM_NOESCAPE	0x01	// Disable backslash escaping.
#define	FNM_PATHNAME	0x02	// Slash must be matched by slash.
#define	FNM_PERIOD		0x04	// Period must be matched by period.
#define	FNM_LEADING_DIR	0x08	// Ignore /<tail> after Imatch.
#define	FNM_CASEFOLD	0x10	// Case insensitive search.
#define FNM_PREFIX_DIRS	0x20	// Directory prefixes of pattern match too.
#define	EOS	        '\0'

//-----------------------------------------------------------------------
static const char * rangematch(const char *pattern, char test, int flags)
{
  int negate, ok;
  char c, c2;

  /*
   * A bracket expression starting with an unquoted circumflex
   * character produces unspecified results (IEEE 1003.2-1992,
   * 3.13.2).  This implementation treats it like '!', for
   * consistency with the regular expression syntax.
   * J.T. Conklin (conklin@ngai.kaleida.com)
   */
  if ( (negate = (*pattern == '!' || *pattern == '^')) ) ++pattern;

  if (flags & FNM_CASEFOLD) test = tolower((unsigned char)test);

  for (ok = 0; (c = *pattern++) != ']';) {
    if (c == '\\' && !(flags & FNM_NOESCAPE)) c = *pattern++;
    if (c == EOS) return (NULL);

    if (flags & FNM_CASEFOLD) c = tolower((unsigned char)c);

    if (*pattern == '-' && (c2 = *(pattern+1)) != EOS && c2 != ']') {
      pattern += 2;
      if (c2 == '\\' && !(flags & FNM_NOESCAPE)) c2 = *pattern++;
      if (c2 == EOS) return (NULL);

      if (flags & FNM_CASEFOLD) c2 = tolower((unsigned char)c2);

      if ((unsigned char)c <= (unsigned char)test &&
          (unsigned char)test <= (unsigned char)c2) ok = 1;
    }
    else if (c == test) ok = 1;
  }
  return (ok == negate ? NULL : pattern);
}

//--------------------------------------------------------------------
static int fnmatch(const char *pattern, const char *string, int flags)
{
  const char *stringstart;
  char c, test;

  for (stringstart = string;;)
    switch (c = *pattern++) {
    case EOS:
      if ((flags & FNM_LEADING_DIR) && *string == '/') return (0);
      return (*string == EOS ? 0 : FNM_NOMATCH);
    case '?':
      if (*string == EOS) return (FNM_NOMATCH);
      if (*string == '/' && (flags & FNM_PATHNAME)) return (FNM_NOMATCH);
      if (*string == '.' && (flags & FNM_PERIOD) &&
          (string == stringstart ||
          ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
              return (FNM_NOMATCH);
      ++string;
      break;
    case '*':
      c = *pattern;
      // Collapse multiple stars.
      while (c == '*') c = *++pattern;

      if (*string == '.' && (flags & FNM_PERIOD) &&
          (string == stringstart ||
          ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
              return (FNM_NOMATCH);

      // Optimize for pattern with * at end or before /.
      if (c == EOS)
        if (flags & FNM_PATHNAME)
          return ((flags & FNM_LEADING_DIR) ||
                    strchr(string, '/') == NULL ?
                    0 : FNM_NOMATCH);
        else return (0);
      else if ((c == '/') && (flags & FNM_PATHNAME)) {
        if ((string = strchr(string, '/')) == NULL) return (FNM_NOMATCH);
        break;
      }

      // General case, use recursion.
      while ((test = *string) != EOS) {
        if (!fnmatch(pattern, string, flags & ~FNM_PERIOD)) return (0);
        if ((test == '/') && (flags & FNM_PATHNAME)) break;
        ++string;
      }
      return (FNM_NOMATCH);
    case '[':
      if (*string == EOS) return (FNM_NOMATCH);
      if ((*string == '/') && (flags & FNM_PATHNAME)) return (FNM_NOMATCH);
      if ((pattern = rangematch(pattern, *string, flags)) == NULL) return (FNM_NOMATCH);
      ++string;
      break;
    case '\\':
      if (!(flags & FNM_NOESCAPE)) {
        if ((c = *pattern++) == EOS) {
          c = '\\';
          --pattern;
        }
      }
      break;
      // FALLTHROUGH
    default:
      if (c == *string) {
      }
      else if ((flags & FNM_CASEFOLD) && (tolower((unsigned char)c) == tolower((unsigned char)*string))) {
      }
      else if ((flags & FNM_PREFIX_DIRS) && *string == EOS && ((c == '/' && string != stringstart) ||
    		  (string == stringstart+1 && *stringstart == '/')))
              return (0);
      else return (FNM_NOMATCH);
      string++;
      break;
    }
  // NOTREACHED
  return 0;
}

// ============================================================================

//-----------------------------------------
void delete_allFiles(char *path ) {
	
    DIR *dir = NULL;
    struct dirent *ent;
    char type;
    char size[9];
    char tpath[255];
    char tbuffer[80];
    struct stat sb;
    struct tm *tm_info;
    char *lpath = NULL;
    int statok;

    printf("LIST of DIR [%s]\r\n", path);
    // Open directory
    dir = opendir(path);
    if (!dir) {
        printf("Error opening directory\r\n");
        return;
    }

    // Read directory entries
    uint64_t total = 0;
    int nfiles = 0;
    printf("T  Size      Date/Time         Name\r\n");
    printf("-----------------------------------\r\n");
    while ((ent = readdir(dir)) != NULL) {
    	sprintf(tpath, path);
        if (path[strlen(path)-1] != '/') strcat(tpath,"/");
        strcat(tpath,ent->d_name);
        tbuffer[0] = '\0';
	// Get file stat
		statok = stat(tpath, &sb);

		if (statok == 0) {
			tm_info = localtime(&sb.st_mtime);
			strftime(tbuffer, 80, "%d/%m/%Y %R", tm_info);
		}
		else sprintf(tbuffer, "                ");

		if (ent->d_type == DT_REG) {
			type = 'f';
			nfiles++;
			if (statok) strcpy(size, "       ?");
			else {
				total += sb.st_size;
				if (sb.st_size < (1024*1024)) sprintf(size,"%8d", (int)sb.st_size);
				else if ((sb.st_size/1024) < (1024*1024)) sprintf(size,"%6dKB", (int)(sb.st_size / 1024));
				else sprintf(size,"%6dMB", (int)(sb.st_size / (1024 * 1024)));
			}
			// delete 
			char fnmdel[30+1];
			sprintf(fnmdel , "%s%s" , path, ent->d_name );
			if( remove(fnmdel ) ==0 ) { 
			   printf("remove =%s\n"  , fnmdel); 
			   return;
			} 
			else {  printf("remove =%s\n"  , fnmdel );  }
		}
		else {
			type = 'd';
			strcpy(size, "       -");
		}

		printf("%c  %s  %s  %s\r\n",
			type,
			size,
			tbuffer,
			ent->d_name
		);

    } //end_while
    
    
    
    closedir(dir);
    free(lpath);
    printf("# delete _all_files #");
}


void list(char *path, char *match) {
	
    DIR *dir = NULL;
    struct dirent *ent;
    char type;
    char size[9];
    char tpath[255];
    char tbuffer[80];
    struct stat sb;
    struct tm *tm_info;
    char *lpath = NULL;
    int statok;

    printf("LIST of DIR [%s]\r\n", path);
    // Open directory
    dir = opendir(path);
    if (!dir) {
        printf("Error opening directory\r\n");
        return;
    }

    // Read directory entries
    uint64_t total = 0;
    int nfiles = 0;
    printf("T  Size      Date/Time         Name\r\n");
    printf("-----------------------------------\r\n");
    while ((ent = readdir(dir)) != NULL) {
    	sprintf(tpath, path);
        if (path[strlen(path)-1] != '/') strcat(tpath,"/");
        strcat(tpath,ent->d_name);
        tbuffer[0] = '\0';

        if ((match == NULL) || (fnmatch(match, tpath, (FNM_PERIOD)) == 0)) {
			// Get file stat
			statok = stat(tpath, &sb);

			if (statok == 0) {
				tm_info = localtime(&sb.st_mtime);
				strftime(tbuffer, 80, "%d/%m/%Y %R", tm_info);
			}
			else sprintf(tbuffer, "                ");

			if (ent->d_type == DT_REG) {
				type = 'f';
				nfiles++;
				if (statok) strcpy(size, "       ?");
				else {
					total += sb.st_size;
					if (sb.st_size < (1024*1024)) sprintf(size,"%8d", (int)sb.st_size);
					else if ((sb.st_size/1024) < (1024*1024)) sprintf(size,"%6dKB", (int)(sb.st_size / 1024));
					else sprintf(size,"%6dMB", (int)(sb.st_size / (1024 * 1024)));
				}
			}
			else {
				type = 'd';
				strcpy(size, "       -");
			}

			printf("%c  %s  %s  %s\r\n",
				type,
				size,
				tbuffer,
				ent->d_name
			);
        }
    }
    if (total) {
        printf("-----------------------------------\r\n");
    	if (total < (1024*1024)) printf("   %8d", (int)total);
    	else if ((total/1024) < (1024*1024)) printf("   %6dKB", (int)(total / 1024));
    	else printf("   %6dMB", (int)(total / (1024 * 1024)));
    	printf(" in %d file(s)\r\n", nfiles);
    }
    printf("-----------------------------------\r\n");

    closedir(dir);

    free(lpath);

	uint32_t tot, used;
	spiffs_fs_stat(&tot, &used);
	printf("SPIFFS: free %d KB of %d KB\r\n", (tot-used) / 1024, tot / 1024);
}

//----------------------------------------------------
static int file_copy(const char *to, const char *from)
{
    FILE *fd_to;
    FILE *fd_from;
    char buf[1024];
    ssize_t nread;
    int saved_errno;

    fd_from = fopen(from, "rb");
    //fd_from = open(from, O_RDONLY);
    if (fd_from == NULL) return -1;

    fd_to = fopen(to, "wb");
    if (fd_to == NULL) goto out_error;

    while (nread = fread(buf, 1, sizeof(buf), fd_from), nread > 0) {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = fwrite(out_ptr, 1, nread, fd_to);

            if (nwritten >= 0) {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR) goto out_error;
        } while (nread > 0);
    }

    if (nread == 0) {
        if (fclose(fd_to) < 0) {
            fd_to = NULL;
            goto out_error;
        }
        fclose(fd_from);

        // Success!
        return 0;
    }

  out_error:
    saved_errno = errno;

    fclose(fd_from);
    if (fd_to) fclose(fd_to);

    errno = saved_errno;
    return -1;
}
//
static int write_ffs(char *buf ,FILE *fd){
	int iRet= mNG_CODE;
	int len = strlen(buf);
	int res = fwrite(buf, 1, len, fd);
	if (res != len) {
    	printf("     Error writing to file(%d <> %d\r\n", res, len);
    	return iRet;
	}
	iRet=mOK_CODE;
	return iRet;
}

//--------------------------------
//void writeTest(char *fname)
int write_alermData(char *fname , char *cTime, char *cTemp)
{
	int iRet= mNG_CODE;
	printf("==== Write to file \"%s\" ====\r\n", fname);

//	int n, res, tot, len;
	int res;
	char buf[40];
	FILE *fd = fopen(fname, "wb");
    if (fd == NULL) {
    	printf("     Error opening file\r\n");
    	return iRet;
    }
    // tot = 0;
    sprintf(buf, "T:%s\n", cTime);
    if(write_ffs(buf ,fd) == mNG_CODE){
		res = fclose(fd);
    	return iRet;
    }
    sprintf(buf, "S:%s\n", cTemp);
    if(write_ffs(buf ,fd) == mNG_CODE){
		res = fclose(fd);
    	return iRet;
    }
    	
	res = fclose(fd);
	if (res) {
    	printf("     Error closing file\r\n");
	}
    printf("\r\n");
    iRet=mOK_CODE;
    return iRet;
}

static int Is_containHead(char *buff  , char *cMatch  , char *retChar){
	int iRet= mNG_CODE;
	char reBuff[30+1];
	int len= strlen( buff );
	int lenMatch =strlen(cMatch );
	int iSt =0;
	for(int i=0; i< len; i++){
		if( buff[i] != 0x0a ){
		    reBuff[ iSt]=buff[i];
    		iSt ++;
		}else{
		    printf("reBuff=%s\n" ,reBuff );
		    iSt=0;
		    if( strncmp(reBuff , cMatch ,lenMatch)==0 ){
			    printf("cmp is OK=%s\n" ,reBuff );
			    strcpy(retChar , reBuff );
			    iRet= mOK_CODE;
			    return iRet;
		    }
		    memset(reBuff, 0x00, 30);
		}
	}
	return iRet;
}
	
	
//-------------------------------
int read_alermData(char *fname, char *sMatch, char *cReturn )
{
    int iRet= mNG_CODE;
	printf("==== Reading from file \"%s\" ====\r\n", fname);

	int res;
	char *buf;
	// char c;
	buf = calloc(1024, 1);
	if (buf == NULL) {
    	printf("     Error allocating read buffer\"\r\n");
    	return iRet;
	}
	FILE *fd = fopen(fname, "rb");
    if (fd == NULL) {
    	printf("     Error opening file\r\n");
    	free(buf);
    	return iRet;
    }
    res = 999;
//    while ( (c=fgetc(fd)) != EOF){
//  	printf("%x",c);
//    }
	res = fread(buf, 1, 1023, fd);
	if (res <= 0) {
		printf("     Error reading from file\r\n");
	}
	else {
		printf("     %d bytes read [\r\n", res);
		buf[res] = '\0';
		printf("%s\r\n]\r\n", buf);
	}
	char retChar[20+1];
	if(Is_containHead( buf  , sMatch  ,  retChar) == mNG_CODE){
	    printf("Error, match");
	}
	printf("retChar=%s \n" ,retChar );
	strcpy( cReturn, retChar );
    retChar[0]= 0x00;
	free(buf);
	printf("\n");
	res = fclose(fd);
	if (res) {
    	printf("     Error closing file\r\n");
	}
	iRet= mOK_CODE;
	return iRet;
}


//-------------------------------
void readTest(char *fname)
{
	printf("==== Reading from file \"%s\" ====\r\n", fname);

	int res;
	char *buf;
	buf = calloc(1024, 1);
	if (buf == NULL) {
    	printf("     Error allocating read buffer\"\r\n");
    	return;
	}

	FILE *fd = fopen(fname, "rb");
    if (fd == NULL) {
    	printf("     Error opening file\r\n");
    	free(buf);
    	return;
    }
    res = 999;
    res = fread(buf, 1, 1023, fd);
    if (res <= 0) {
    	printf("     Error reading from file\r\n");
    }
    else {
    	printf("     %d bytes read [\r\n", res);
        buf[res] = '\0';
        printf("%s\r\n]\r\n", buf);
    }
	free(buf);

	res = fclose(fd);
	if (res) {
    	printf("     Error closing file\r\n");
	}
    printf("\r\n");
}

//----------------------------------
static void mkdirTest(char *dirname)
{
	printf("==== Make new directory \"%s\" ====\r\n", dirname);

	int res;
	struct stat st = {0};
	char nname[80];

	if (stat(dirname, &st) == -1) {
	    res = mkdir(dirname, 0777);
	    if (res != 0) {
	    	printf("     Error creating directory (%d)\r\n", res);
	        printf("\r\n");
	        return;
	    }
    	printf("     Directory created\r\n\r\n");
		list("/spiffs/", NULL);
		vTaskDelay(1000 / portTICK_RATE_MS);

    	printf("     Copy file from root to new directory...\r\n");
    	sprintf(nname, "%s/test.txt.copy", dirname);
    	res = file_copy(nname, "/spiffs/test.txt");
	    if (res != 0) {
	    	printf("     Error copying file (%d)\r\n", res);
	    }
    	printf("\r\n");
    	list(dirname, NULL);
		vTaskDelay(1000 / portTICK_RATE_MS);

    	printf("     Removing file from new directory...\r\n");
	    res = remove(nname);
	    if (res != 0) {
	    	printf("     Error removing directory (%d)\r\n", res);
	    }
    	printf("\r\n");
    	list(dirname, NULL);
		vTaskDelay(1000 / portTICK_RATE_MS);

    	printf("     Removing directory...\r\n");
	    res = remove(dirname);
	    if (res != 0) {
	    	printf("     Error removing directory (%d)\r\n", res);
	    }
    	printf("\r\n");
		list("/spiffs/", NULL);
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
	else {
		printf("     Directory already exists, removing\r\n");
	    res = remove(dirname);
	    if (res != 0) {
	    	printf("     Error removing directory (%d)\r\n", res);
	    }
	}

    printf("\r\n");
}

