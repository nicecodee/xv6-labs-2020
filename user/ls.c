#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];   //DIRSIZ is defined to be 14 in `kernel/fs.h`
  char *p;

  // Find first character after last slash（p首先指向path字符串的最后一个字符，然后从右往左找到第一个"/" ，找到后退出for循环）
  for(p=path+strlen(path); p >= path && *p != '/'; p--) 
    ;
  p++;  // 此时 p 指向最后一个 "/" 后面的字符串，即文件名。例如当路径为"a/b/cd"时，此时p指向字符串"cd"

  // Return blank-padded name. （如果 p 指向的字符串的长度大于 DIRSIZ，直接返回 p）
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));  //void *memmove(void *str1, const void *str2, size_t n) : copies n bytes from str2 to str1, and returns str1.
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));  // 如果 p 的长度小于 DIRSIZ，则用空格补齐
  return buf;
}

void
ls(char *path)
{
  char buf[512], *p;  // buf 用来存储路径
  int fd;
  struct dirent de;   //struct dirent is defined in fs.h. Its items include： inum and name[DIRSIZ]
  struct stat st;     //struct stat is defined in stat.h, Its items include： dev,ino,type,nlink and size

  if((fd = open(path, 0)) < 0){ // the "0" argument in open() indicates read data with the "O_RDONLY" flag
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){  //int fstat(int fd, struct stat *buf) :  调用 fstat 将 fd 对应的文件信息记录在 st 中
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){  // st.type is defined in stat.h and it includes 1:T_DIR, 2:T_FILE and 3:T_DEVICE
  case T_FILE:  // st.type 为文件时直接返回即可
    printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);  
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){ // 限制路径 path 的长度防止缓存溢出
      printf("ls: path too long\n");
      break;
    }
     // 将当前路径存入 buf 中，并在最后加入 '/' 为重复构建子路径作准备
    // 注意，完成以下3行操作后， buf 包括：当前路径 + '/'，p 指向 '/' 的下一位
    strcpy(buf, path);  // copy the string pointed by path (including the null character) to the buf.
    p = buf+strlen(buf);   
    *p++ = '/';   
    while(read(fd, &de, sizeof(de)) == sizeof(de)){ //read the fd by the size of de (i.e. the size of one dir entry) each time, the subsequent read will start from the offset of last read
      if(de.inum == 0)  //de.inum==0 indicates no file exists in this dir. inum == 0 说明无效文件或目录
        continue;
      /*
       * de.name 表示该文件的文件名（可能是 "."（当前目录）， ".."（上层目录））
       * 这里主要是将 de 的 name 复制到 p 指向的位置，与当前路径合并作为该文件的完整路径，并在最后加 '0' 构建字符串
       * 注意由于这里 p 的位置没有变动，所以下一次循环重复复制后就可以保持当前路径只更新文件名了
       * 这也是为什么末尾要补 '0' 提示字符串终止位置的原因之一。
       */
      memmove(p, de.name, DIRSIZ); //void *memmove(void *str1, const void *str2, size_t n) : // 将 de.name 拷贝到 p 中
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
