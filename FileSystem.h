#pragma once
#define Block_Size 512     //每一块大小为512B
#define Block_Count 2048
#define ERROR -1
#define OK 1
#include<fstream>
#include<iostream>
#include<string>
struct Block          //8B
{
	int start_pos;   //起始地址
	int next_num;  //链表的下一块编号
};

struct FAT               //2048*8/512=占用32块block
{
	Block blocktable[Block_Count];
};

class SuperBlock       //SuperBlock占用0#物理盘块，136B
{
public:
	int c_inode;      //inode占用的盘块数
	int c_fat;           //fat区占用盘块数
	int c_data;		   //文件数据区占用的盘块数
	int c_total;		   //盘块总数
	int start_inode;           //inode区的起始地址
	int start_fat;                //FAT区的起始地址
	int inode_root_dir;        //根目录的inode地址
	int start_data;             //文件数据区的起始地址
	int f_data_count;        //文件数据区空闲的盘块数
	int f_data_entry;         //空闲盘块入口地址
	int f_data_exit;            //空闲盘块出口地址
	int f_inode_count;       //空闲的inode数；
	short int f_inode[120];   //直接管理的120个空闲的inode
public:
	void format();
};

class inode             //一个文件对应一个inode，每个inode64个字节
{
public:
	int occupied_block[10];   
	int filesize;
	int type;               //文件类型：0-普通数据文件；1目录文件
	int filenum;           //文件标识号
	int mode;             //0表示关闭，1表示只读方式打开，2表示只写方式打开，3表示可读可写方式打开
	int padding[2];
public:
	inode();
};

class Directory        //32B
{
public:
	int inode;
	char filename[28];
};

class File
{
public:
	int mode;
	int inode_addr;
	int flseek;          //文件读写指针
	File();
};

class FileSystem
{
public:
	std::fstream iofile;   //文件操作的全局变量
	std::string path;
	File *file[10];
	SuperBlock SB;
	FAT fat;
public:
	FileSystem(const char* pPath, std::ios_base::openmode mode = std::ios::out | std::ios::in | std::ios::binary) :path(pPath), iofile(pPath,mode) {
		for (int i = 0; i < 10; i++)
		{
			file[i] = new File;
			file[i]->mode = 0;
			file[i]->flseek = 0;
		}
	};
	void fformat();
	void ls();
	//void initiate();
	int AllocFile();
	int AllocBlock();
	int AllocInode();
	void RecycleBlock(int block_addr);
	void RecycleInode(int inode_addr);
	void loadsuperblock();            //加载superblock
	void savesuperblock();
	void loadFAT();                      //加载FAT区
	void saveFAT();
	int  fopen(char name[], int mode);	             //打开文件
	void fclose(int fd);                                      //关闭文件
	int fread(int fd, char *buffer, int length);     //读文件
	int fwrite(int fd, char *buffer, int length);    //写文件
	int flseek(int fd, int position);                     //定位文件读写指针
	int fcreat(char *name);              //新建文件
	int fdelete(char *name);                            //删除文件
};