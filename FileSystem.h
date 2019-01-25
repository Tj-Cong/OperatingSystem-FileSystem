#pragma once
#define Block_Size 512     //ÿһ���СΪ512B
#define Block_Count 2048
#define ERROR -1
#define OK 1
#include<fstream>
#include<iostream>
#include<string>
struct Block          //8B
{
	int start_pos;   //��ʼ��ַ
	int next_num;  //�������һ����
};

struct FAT               //2048*8/512=ռ��32��block
{
	Block blocktable[Block_Count];
};

class SuperBlock       //SuperBlockռ��0#�����̿飬136B
{
public:
	int c_inode;      //inodeռ�õ��̿���
	int c_fat;           //fat��ռ���̿���
	int c_data;		   //�ļ�������ռ�õ��̿���
	int c_total;		   //�̿�����
	int start_inode;           //inode������ʼ��ַ
	int start_fat;                //FAT������ʼ��ַ
	int inode_root_dir;        //��Ŀ¼��inode��ַ
	int start_data;             //�ļ�����������ʼ��ַ
	int f_data_count;        //�ļ����������е��̿���
	int f_data_entry;         //�����̿���ڵ�ַ
	int f_data_exit;            //�����̿���ڵ�ַ
	int f_inode_count;       //���е�inode����
	short int f_inode[120];   //ֱ�ӹ����120�����е�inode
public:
	void format();
};

class inode             //һ���ļ���Ӧһ��inode��ÿ��inode64���ֽ�
{
public:
	int occupied_block[10];   
	int filesize;
	int type;               //�ļ����ͣ�0-��ͨ�����ļ���1Ŀ¼�ļ�
	int filenum;           //�ļ���ʶ��
	int mode;             //0��ʾ�رգ�1��ʾֻ����ʽ�򿪣�2��ʾֻд��ʽ�򿪣�3��ʾ�ɶ���д��ʽ��
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
	int flseek;          //�ļ���дָ��
	File();
};

class FileSystem
{
public:
	std::fstream iofile;   //�ļ�������ȫ�ֱ���
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
	void loadsuperblock();            //����superblock
	void savesuperblock();
	void loadFAT();                      //����FAT��
	void saveFAT();
	int  fopen(char name[], int mode);	             //���ļ�
	void fclose(int fd);                                      //�ر��ļ�
	int fread(int fd, char *buffer, int length);     //���ļ�
	int fwrite(int fd, char *buffer, int length);    //д�ļ�
	int flseek(int fd, int position);                     //��λ�ļ���дָ��
	int fcreat(char *name);              //�½��ļ�
	int fdelete(char *name);                            //ɾ���ļ�
};