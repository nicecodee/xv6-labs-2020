#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash from the right end of the string "path"(from right to left)
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));  //void *memmove(void *str1, const void *str2, size_t n) : copies n bytes from str2 to str1, and returns str1.
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));  //void *memset(void *str, int c, size_t n): copies the character c (an unsigned char) to the first n bytes of the str pointed to.
  return buf;
}

void
ls(char *path)
{
  char buf[512], *p;
  int fd;
  struct dirent de;   //struct dirent is defined in fs.h. Its items include： inum and name[DIRSIZ]
  struct stat st;     //struct stat is defined in stat.h, Its items include： dev,ino,type,nlink and size

  if((fd = open(path, 0)) < 0){ // the "0" argument in open() indicates read data with the "O_RDONLY" flag
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){  //int fstat(int fd, struct stat *buf) : get file info based on its file descriptor. It returns a negative value on failure.
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){  // st.type is defined in stat.h and it includes 1:T_DIR, 2:T_FILE and 3:T_DEVICE
  case T_FILE:
    printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){  //DIRSIZ is defined to be 14 in fs.h
      printf("ls: path too long\n");
      break;
    }
    strcpy(buf, path);  // copy the string pointed by path (including the null character) to the buf.
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){ // ssize_t read(int fd, void *buf, size_t nbytes): read data from fd into a buffer.
      if(de.inum == 0)  //0 indicates no inode. Most file system by convention use #1 as the root inode's  inode number. 
        continue;
      memmove(p, de.name, DIRSIZ); //void *memmove(void *str1, const void *str2, size_t n) : copies n bytes from str2 to str1, and returns str1.
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){  //int stat(const char *path, struct stat *buf): get file info based on its file path. It returns a negative value on failure.
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
      printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".");
    exit(0);
  }
  for(i=1; i<argc; i++)
    ls(argv[i]);
  exit(0);
}
