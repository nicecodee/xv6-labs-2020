#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void
find(char *path, char *filename)
{
	char buf[512], *p;  ///buf 用来存储路径
	int fd;
	struct dirent de;   //struct dirent is defined in fs.h. Its items include： inum and name[DIRSIZ]
	struct stat st;     //struct stat is defined in stat.h, Its items include： dev,ino,type,nlink and size
	// 打开文件路径，并判断是否成功
	if((fd = open(path, 0)) < 0){ // the "0" argument in open() indicates read data with the "O_RDONLY" flag
		fprintf(2, "find: cannot open %s\n", path);
		return;
	}
	// 读取文件信息
	if(fstat(fd, &st) < 0){   
		fprintf(2, "find: cannot stat %s\n", path);
		close(fd);
		return;
	}
	// 判断该路径是文件还是文件夹
	switch(st.type){   
		case T_FILE:  // 如果是文件，直接报错跳出，因为我们要递归查询的是目录
			fprintf(2, "find: %s is not a directory!\n", path);
			break; 
	
		case T_DIR:  // 假如是文件夹
			// 路径过长退出
			if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
			  printf("find: path too long\n");
			  break;
			}
			// 将当前路径存入 buf 中，并在最后加入 '/' 为重复构建子路径作准备
			// 注意，完成以下3行操作后， buf 包括：当前路径 + '/'，p 指向 '/' 的下一位
			strcpy(buf, path);   
			p = buf+strlen(buf);    
			*p++ = '/';    
			
			 // 依次读取 struct dirent, 每一个 de 代表文件夹下的一个文件，直到读不了为止
			 // de.name 表示该文件的文件名（可能是 "."（当前目录）或 ".."（上层目录），遇到就跳过，不然就会无限循环了） 
			while(read(fd, &de, sizeof(de)) == sizeof(de)){
				if(de.inum == 0 ||  strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)  
					continue;  
				/*
		       * 这里主要是将 de 的 name 复制到 p 指向的位置，与当前路径合并作为该文件的完整路径，并在最后加 '0' 构建字符串
		       * 注意由于这里 p 的位置没有变动，所以下一次循环重复复制前，要把p原来指向的字符串及其后面的'/0'用空格清空，就可以保持当前路径只更新文件名了
		       */
				memset(p, ' ', strlen(de.name)+1);
				memmove(p, de.name, strlen(de.name));
				p[strlen(de.name)] = 0;
				
				if(stat(buf, &st) < 0){  
					printf("find: cannot stat %s\n", buf);
					continue;   // 记住用continue 返回 while循环，继续读取后面的de
				}
				// 遇到目录类型，递归查找 
				if (st.type == T_DIR){  
					find(buf, filename);
					continue;   // 记住用continue 返回 while循环，继续读取后面的de 
				}else{
					if(strcmp(p, filename) == 0){
						printf("%s\n", buf);
					}
					continue;  // 记住用continue 返回 while循环，继续读取后面的de
				}
				break;
			}
			close(fd);
	}
}

int
main(int argc, char *argv[])
{
	if(argc != 3){
		fprintf(2, "Usage: find <dir> <filename>\n");
		exit(1);
	}
	find(argv[1], argv[2]);
	exit(0);
}

