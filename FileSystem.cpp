#include"Queue.h"
using namespace std;

void SuperBlock::format()
{
	c_inode=15;      //inode占用的盘块数
	c_fat=32;
	c_data=2000;		   //文件数据区占用的盘块数
	c_total=2048;		   //盘块总数
	start_inode=512;           //inode区的起始地址
	start_fat=8192;                //FAT区的起始地址 16*512
	inode_root_dir = 512;
	start_data=24576;             //文件数据区的起始地址48*512
	f_data_count=1999;        //文件数据区空闲的盘块数,第一块已分配给根目录
	f_data_entry=49;         //空闲盘块入口地址, 48#已分配给根目录
	f_data_exit = 2047;
	f_inode_count=119;       //空闲的inode数；
	for (int i = f_inode_count-1; i >= 0; i--)
	{
		f_inode[i] = 512 + 64 * (119 - i);
	}
}

inode::inode()
{
	for (int i = 0; i < 10; i++)
	{
		occupied_block[i] = -1;             //表示未分配任何盘块
	}
	filesize = 0;
	type = 0;
	mode = 0;
	for (int j = 0; j < 1; j++)
		padding[j] = 0;
}

File::File()
{
	mode = 0;
	inode_addr = 0;
	flseek = 0;
}

void FileSystem::fformat()
{
	SuperBlock SB;
	SB.format();
	iofile.seekp(0);
	iofile.write((char*)&SB, sizeof(SB));    //Superblock 格式化完毕

	/*共120个inode，其中第一个inode是根目录文件的inode*/
	inode *DiskInode = new inode[120];     //定义120个inode
	for (int i = 0; i < 120; i++)
	{
		DiskInode[i].filenum = i;
		for (int j = 0; j < 10; j++)
			DiskInode[i].occupied_block[j] = 0;
	}
	DiskInode[0].occupied_block[0] = 24576;
	DiskInode[0].type = 1;                         //目录文件
	iofile.seekp(512);
	iofile.write((char*)&DiskInode[0], 64*120);  //格式化inode完毕

	/*共2048个FAT*/
	FAT *fat=new FAT;
	for (int i = 0; i < Block_Count; i++)
	{
		fat->blocktable[i].start_pos = 512 * i;   //第i个盘块的起始地址为512*i；
		fat->blocktable[i].next_num = -1;         //还未成链
	}
	for (int j = 49; j < Block_Count - 1; j++)
	{
		fat->blocktable[j].next_num = j + 1;
	}
	iofile.seekp(8192);
	iofile.write((char*)fat, sizeof(*fat));    //FAT区格式化完毕
}

int FileSystem::AllocFile()
{
	int i = 0;
	for (i; i < 10; i++)
	{
		if (file[i]->mode == 0)
			return i;
	}
	if (i == 10)
	{
		cout << "最多打开10个文件！" << endl;
		return ERROR;
	}
}

int FileSystem::AllocBlock()
{
	if (SB.f_data_count <= 0)
	{
		cout << "警告：磁盘空间不足！" << endl;
		return ERROR;
	}
	int entry = SB.f_data_entry;
	int addr = fat.blocktable[entry].start_pos;
	SB.f_data_entry = fat.blocktable[entry].next_num;
	fat.blocktable[entry].next_num = -1;
	SB.f_data_count--;
	savesuperblock();
	saveFAT();
	return addr;
}
void FileSystem::RecycleBlock(int block_addr)
{
	int block_num = block_addr / 512;
	int exit = SB.f_data_exit;
	fat.blocktable[exit].next_num = block_num;
	fat.blocktable[block_num].next_num = -1;
	SB.f_data_count++;
	SB.f_data_exit = block_num;
	savesuperblock();
	saveFAT();
}

int FileSystem::AllocInode()
{
	if (SB.f_inode_count <= 0)
	{
		cout << "超过文件最大数，不可再创建文件！" << endl;
		return ERROR;
	}
	int addr=SB.f_inode[--SB.f_inode_count];
	savesuperblock();
	return addr;
}

void FileSystem::RecycleInode(int inode_addr)
{
	SB.f_inode[SB.f_inode_count++] = inode_addr;
	savesuperblock();
}

void FileSystem::loadsuperblock()
{
	iofile.seekg(0);
	iofile.read((char*)&SB, sizeof(SB));
}

void FileSystem::savesuperblock()
{
	iofile.seekp(0);
	iofile.write((char*)&SB, sizeof(SB));
}

void FileSystem::loadFAT()
{
	iofile.seekg(8192);
	iofile.read((char*)&fat, sizeof(fat));
}

void FileSystem::saveFAT()
{
	iofile.seekp(8192);
	iofile.write((char*)&fat, sizeof(fat));
}

int  FileSystem::fopen(char name[], int mode)	             //打开文件
{
	if (mode != 1 && mode != 2 && mode != 3)
	{
		cout << "错误的打开方式！mode={1(只读),2(只写),3(可读可写)}" << endl;
		return (-1);
	}
	char p;
	char *filename=new char[28];
	int fnptr = 0;
	int inode_addr = 512;
	inode in;

	int i = 0;
	p = name[i];
	while (p != '/' && p!='\0')
	{
		p = name[++i];
	}

	for (i=i+1; p != '\0'; i++)
	{
		p = name[i];
		if (p != '/' && p != '\0')
		{
			filename[fnptr++] = p;
		}
		else
		{
			filename[fnptr] = '\0';
			iofile.seekg(inode_addr);
			iofile.read((char*)&in, sizeof(in));     //读出inode
			int dict_count = in.filesize / 32;      //目录条数=文件大小/32
			int block_count = in.filesize / 512;
			Directory *dict=new Directory[dict_count];
			int j = 0;
			for (j; j < block_count; j++)
			{
				iofile.seekg(in.occupied_block[j]);
				iofile.read((char*)&dict[j * 16], 512);
			}
			if (in.filesize - block_count * 512)
			{
				iofile.seekg(in.occupied_block[j]);
				iofile.read((char*)&dict[j * 16], in.filesize - block_count * 512);
			}
			int k = 0;
			for (k; k < dict_count; k++)
			{
				if (strcmp(filename, dict[k].filename) == 0)
				{
					inode_addr = dict[k].inode;
					fnptr = 0;
					delete [] filename;
					filename = new char[28];
					break;
				}
			}
			if (k == dict_count)
			{
				cout << "open error！文件不存在！" << endl;
				return -1;
			}
		}
	}
	
	int filenum = AllocFile();
	file[filenum]->mode = mode;
	file[filenum]->inode_addr = inode_addr;
	file[filenum]->flseek = 0;

	int m = mode;
	iofile.seekp(inode_addr + 52);
	iofile.write((char*)&m, 4);

	iofile.seekp(inode_addr + 48);
	iofile.write((char*)&filenum, 4);
	return filenum;
}

void FileSystem::fclose(int fd)
{
	if (fd < 0)
	{
		cout << "error！错误的文件号！" << endl;
		return;
	}
	if (file[fd]->mode == 0)
	{
		cout << "error！错误的文件号：" << fd << endl;
		return;
	}
	int addr = file[fd]->inode_addr + 52;
	int m = 0;
	iofile.seekp(addr);
	iofile.write((char*)&m, 4);
	delete file[fd];
	file[fd] = new File;
}

int FileSystem::fread(int fd, char *buffer, int length)     //读文件
{
	int m = file[fd]->mode;
	int filesize, addr;
	addr = file[fd]->inode_addr + 40;
	iofile.seekg(addr);
	iofile.read((char*)&filesize, 4);
	switch (m)
	{
	case 0:cout << "error！错误的文件号：" << fd << endl; break;
	case 1:
	case 3: {
		if (file[fd]->flseek + length > filesize)
		{
			cout << "读取错误！超出文件大小！请检查读写指针和读取长度（每次读写后，读写指针都会改变）！" << endl;
			return -1;
		}
		char *buff = new char[length + 1];
		buff[length] = '\0';
		int start_block = file[fd]->flseek / 512;
		int block_num[10], buffptr = 0, left_block_count, u, v;
		left_block_count = (length > (512 - file[fd]->flseek % 512)) ? ((length - (512 - file[fd]->flseek % 512)) / 512) : 0;
		u = (length > (512 - file[fd]->flseek % 512)) ? (512 - file[fd]->flseek % 512) : (length);
		v = (length > (512 - file[fd]->flseek % 512)) ? (length - u) % 512 : 0;

		iofile.seekg(file[fd]->inode_addr);
		iofile.read((char*)&block_num[0], 40);

		/*处理第一块*/
		iofile.seekg(block_num[start_block]+(file[fd]->flseek%512));
		if (length <= (512 - file[fd]->flseek % 512))      //不跨块
		{
			iofile.read(buff, length);
			strcpy(buffer, buff);
			return OK;
		}
		else
		{
			iofile.read((char*)&buff[0], u);
			buffptr += u;
			/*处理中间块*/
			for (int i = 1; i <= left_block_count; i++)
			{
				iofile.seekg(block_num[start_block + i]);
				iofile.read((char*)&buff[buffptr], 512);
				buffptr += 512;
			}

			/*处理最后一块*/
			iofile.seekg(block_num[start_block + left_block_count + 1]);
			iofile.read((char*)&buff[buffptr], v);
			strcpy(buffer, buff);
			return OK;
		}
		file[fd]->flseek += length;
		break; 
	}
	case 2:cout << "error！无读取权限，请以读方式或读写方式打开！" << endl; break;
	
	default:
		break;
	}
}

int FileSystem::flseek(int fd, int position)                     //定位文件读写指针
{
	if (fd < 0)
	{
		cout << "error！错误的文件号！" << endl;
		return ERROR;
	}
	if (file[fd]->mode == 0)
	{
		cout << "error！错误的文件号：" << fd << endl;
		return ERROR;
	}
	file[fd]->flseek = position;
	return OK;
}

int FileSystem::fwrite(int fd, char *buffer, int length)    //写文件,修改filesize；
{
	if (fd < 0)
	{
		cout << "error！错误的文件号！" << endl;
		return ERROR;
	}
	int mode = file[fd]->mode;
	switch (mode)
	{
	case 0:cout << "error！错误的文件号：" << fd << endl; break;
	case 1:cout << "文件无写权限，请以写方式或读写方式打开文件！" << endl; break;
	case 2:
	case 3: {
		int inode_addr = file[fd]->inode_addr;
		inode in;
		iofile.seekg(inode_addr);
		iofile.read((char*)&in, sizeof(in));    //加载inode
		if (file[fd]->flseek + length > 5120)
		{
			cout << "写入失败！超过文件最大范围(5120B)，请检查读写指针和写入长度（每次读写后，读写指针都会改变）！" << endl;
			return ERROR;
		}
		int existing_block, need_block, need_length, alloc_count;     //已经分配的盘块数；
		existing_block = (in.filesize==0)?1:(in.filesize / 512 + (in.filesize % 512) ? 1 : 0);
		need_length = file[fd]->flseek + length;
		int i = (need_length % 512) ? 1 : 0;
		need_block = need_length / 512 + i;
		alloc_count = need_block - existing_block;
		for (int i = existing_block; i < need_block; i++)
		{
			int blockaddr = AllocBlock();
			in.occupied_block[i] = blockaddr;
		}     //空间分配结束
		in.filesize = need_length;
		iofile.seekp(file[fd]->inode_addr);
		iofile.write((char*)&in, sizeof(in));

		int start_block = file[fd]->flseek / 512;
		int buffptr = 0, left_block_count, u, v;
		left_block_count = (length >(512 - file[fd]->flseek % 512)) ? ((length - (512 - file[fd]->flseek % 512)) / 512) : 0;
		u = (length > (512 - file[fd]->flseek % 512)) ? (512 - file[fd]->flseek % 512) : (length);
		v = (length > (512 - file[fd]->flseek % 512)) ? (length - u) % 512 : 0;
		
		iofile.seekp(in.occupied_block[start_block] + (file[fd]->flseek % 512));
		
		/*写第一块*/
		if (length > (512 - file[fd]->flseek % 512))      //跨块
		{
			iofile.write((char*)&buffer[0], u);
			buffptr += u;
			for (int i = 1; i <= left_block_count; i++)
			{
				iofile.seekp(in.occupied_block[start_block + i]);
				iofile.write((char*)&buffer[buffptr], 512);
				buffptr += 512;
			}
			iofile.seekp(in.occupied_block[start_block + left_block_count + 1]);
			iofile.write((char*)&buffer[buffptr], v);
			cout << "写入成功！" << endl;
			file[fd]->flseek += length;
			return OK;
		}
		else {            //不跨块
			iofile.write((char*)&buffer[0], length);
			cout << "写入成功！" << endl;
			file[fd]->flseek += length;
			return OK;
		}
		break;
	}
	default:
		break;
	}
}

int FileSystem::fcreat(char *name)              //新建文件
{
	char p;
	char *filename = new char[28];
	int fnptr = 0;
	int inode_addr = 512;
	inode in;

	int i = 0;
	p = name[i];
	while (p != '/' && p != '\0')
	{
		p = name[++i];
	}                                          //跳过root

	for (i = i + 1; p != '\0'; i++)
	{
		p = name[i];
		if (p != '/' && p != '\0')
		{
			filename[fnptr++] = p;
		}
		else                                           //断词
		{
			filename[fnptr] = '\0';
			iofile.seekg(inode_addr);
			iofile.read((char*)&in, sizeof(in));     //读出inode
			int dict_count = in.filesize / 32;      //目录条数=文件大小/32
			if (dict_count == 0)   //空文件夹
			{
				/*新建目录项*/
				Directory dic;
				strcpy(dic.filename, filename);
				dic.inode = AllocInode();
				in.filesize += 32;
				iofile.seekp(in.occupied_block[0]);
				iofile.write((char*)&dic, sizeof(dic));      //新建目录项完成
				iofile.seekp(inode_addr);
				iofile.write((char*)&in, sizeof(in));         //in.filesize有修改，重新写回磁盘
				inode_addr = dic.inode;                      //为新目录文件创建inode
				inode ii;
				ii.filesize = 0;
				ii.occupied_block[0] = AllocBlock();
				ii.type = 1;
				iofile.seekp(inode_addr);
				iofile.write((char*)&ii, sizeof(ii));

				fnptr = 0;
				delete[] filename;
				filename = new char[28];
			}
			else
			{
				int block_count = in.filesize / 512;
				Directory *dict = new Directory[dict_count];
				int j = 0, k = 0;
				bool find = false;
				for (j; j < block_count; j++)                         //先把整块的扫描完
				{
					k = j * 16;
					iofile.seekg(in.occupied_block[j]);
					iofile.read((char*)&dict[k], 512);			      //加载第一块目录项		
					for (int m=0; m < 16; m++)
					{
						if (strcmp(dict[k].filename, filename) == 0)     //定位成功
						{
							find = true;
							inode_addr = dict[k].inode;
							fnptr = 0;
							delete[] filename;
							filename = new char[28];
							break;
						}
						k++;
					}
				}
				if (find)
					continue;
				if (in.filesize - block_count * 512)
				{
					iofile.seekg(in.occupied_block[j]);
					iofile.read((char*)&dict[j * 16], in.filesize - block_count * 512);
					for (k = j * 16; k < dict_count; k++)
					{
						if (strcmp(dict[k].filename, filename) == 0)     //定位成功
						{
							find = true;
							inode_addr = dict[k].inode;
							fnptr = 0;
							delete[] filename;
							filename = new char[28];
							break;
						}
					}
					if (find == false)                                                 //目录不存在
					{
						/*新建目录项*/
						Directory dic;
						strcpy(dic.filename, filename);
						dic.inode = AllocInode();
						iofile.seekp(in.occupied_block[j]+in.filesize%512);
						iofile.write((char*)&dic, sizeof(dic));      //新建目录项完成
						in.filesize += 32;
						iofile.seekp(inode_addr);
						iofile.write((char*)&in, sizeof(in));         //in.filesize有修改，重新写回磁盘
						inode_addr = dic.inode;                      //为新目录文件创建inode
						inode ii;
						ii.filesize = 0;
						ii.occupied_block[0] = AllocBlock();
						ii.type = 1;
						iofile.seekp(inode_addr);
						iofile.write((char*)&ii, sizeof(ii));

						fnptr = 0;
						delete[] filename;
						filename = new char[28];
					}
				}
				else if (find==false)
				{
					/*新建目录项*/
					Directory dic;
					strcpy(dic.filename, filename);
					dic.inode = AllocInode();
					in.occupied_block[j] = AllocBlock();
					iofile.seekp(in.occupied_block[j]);
					iofile.write((char*)&dic, sizeof(dic));      //新建目录项完成
					in.filesize += 32;
					iofile.seekp(inode_addr);
					iofile.write((char*)&in, sizeof(in));         //in.filesize有修改，重新写回磁盘
					inode_addr = dic.inode;                      //为新目录文件创建inode
					inode ii;
					ii.filesize = 0;
					ii.occupied_block[0] = AllocBlock();
					ii.type = 1;
					iofile.seekp(inode_addr);
					iofile.write((char*)&ii, sizeof(ii));

					fnptr = 0;
					delete[] filename;
					filename = new char[28];
				}
			}
		}
	}
	int type = 0;
	iofile.seekp(inode_addr + 44);
	iofile.write((char*)&type, 4);
	return inode_addr;
}

int FileSystem::fdelete(char *name)                            //删除文件
{
	char p;
	char *filename = new char[28];
	int fnptr = 0;
	int inode_addr = 512;
	inode in;

	int i = 0;
	p = name[i];
	while (p != '/' && p != '\0')
	{
		p = name[++i];
	}                                             //跳过root

	for (i = i + 1; p != '\0'; i++)
	{
		p = name[i];
		if (p != '/' )
		{
			filename[fnptr++] = p;
		}
		else                                                //断词成功
		{
			filename[fnptr] = '\0';
			iofile.seekg(inode_addr);
			iofile.read((char*)&in, sizeof(in));     //读出inode
			int dict_count = in.filesize / 32;      //目录条数=文件大小/32
			int block_count = in.filesize / 512;
			Directory *dict = new Directory[dict_count];
			int j = 0;
			for (j; j < block_count; j++)
			{
				iofile.seekg(in.occupied_block[j]);
				iofile.read((char*)&dict[j * 16], 512);
			}
			if (in.filesize - block_count * 512)
			{
				iofile.seekg(in.occupied_block[j]);
				iofile.read((char*)&dict[j * 16], in.filesize - block_count * 512);
			}                                                 //目录加载完成
			int k = 0;
			for (k; k < dict_count; k++)                            //目录项扫描
			{
				if (strcmp(filename, dict[k].filename) == 0)
				{
					inode_addr = dict[k].inode;
					fnptr = 0;
					delete[] filename;
					filename = new char[28];
					break;
				}
			}
			if (k == dict_count)
			{
				cout << "error！文件不存在！" << endl;
				return -1;
			}
		}
	}
	/*处理最后一级目录，要在最后一级目录中删除相应目录项*/
	iofile.seekg(inode_addr);
	iofile.read((char*)&in, sizeof(in));     //读出inode
	int dict_count = in.filesize / 32;      //目录条数=文件大小/32
	int block_count = in.filesize / 512;
	Directory *dict = new Directory[dict_count];
	int j = 0;
	for (j; j < block_count; j++)
	{
		iofile.seekg(in.occupied_block[j]);
		iofile.read((char*)&dict[j * 16], 512);
	}
	if (in.filesize - block_count * 512)
	{
		iofile.seekg(in.occupied_block[j]);
		iofile.read((char*)&dict[j * 16], in.filesize - block_count * 512);
	}                                                 //目录加载完成
	
	if (strcmp(filename, dict[dict_count - 1].filename) == 0)    //目录最后一项是要找的目录
	{
		in.filesize -= 32;
		if (in.filesize % 512 == 0 && in.filesize!=0)
		{
			in.occupied_block[in.filesize / 512] = -1;
			RecycleBlock(in.occupied_block[in.filesize / 512]);
		}
		iofile.seekp(inode_addr);
		iofile.write((char*)&in, sizeof(in));       //更新inode
		inode_addr = dict[dict_count-1].inode;
	}
	else
	{
		int k = 0;
		for (k; k < dict_count-1; k++)                            //目录项扫描
		{
			if (strcmp(filename, dict[k].filename) == 0)
			{
				in.filesize -= 32;
				int k_addr = in.occupied_block[k / 16] + 32 * (k % 16);
				iofile.seekp(k_addr);
				iofile.write((char*)&dict[dict_count-1], 32);
				if (in.filesize % 512 == 0)
				{
					in.occupied_block[in.filesize / 512] = -1;
					RecycleBlock(in.occupied_block[in.filesize / 512]);
				}
				iofile.seekp(inode_addr);
				iofile.write((char*)&in, sizeof(in));       //更新inode
				inode_addr = dict[k].inode;				
				break;
			}
		}
		if (k == dict_count-1)
		{
			cout << "error！文件不存在！" << endl;
			return -1;
		}
	}

	/*删除最终文件的inode,回收分配给它的block*/
	iofile.seekg(inode_addr);
	iofile.read((char*)&in, sizeof(in));
	if (in.mode != 0 && file[in.filenum]->inode_addr==inode_addr)
	{
		delete file[in.filenum];
		file[in.filenum] = new File;
	}
	for (int i = 0; i < 10; i++)
	{
		if (in.occupied_block[i] != -1)
			RecycleBlock(in.occupied_block[i]);
	}
	RecycleInode(inode_addr);
	return OK;
}

void FileSystem::ls()                     //列目录
{
	cout << "1级目录（&文件）：" << endl;
	cout << "root" << endl;
	cout << endl;
	int diclevel = 2;
	Queue inode_queue;
	QElem q, p;
	inode in;
	char f[] = "root";
	strcpy(q.filename, f);
	q.inode_addr = 512;
	inode_queue.insert(q);
	while (inode_queue.fetch(p) != ERROR)
	{
		iofile.seekg(p.inode_addr);
		iofile.read((char*)&in, sizeof(in));
		if (in.type == 1)                                  //判断是否为目录文件，若不是则continue
		{
			int dict_count = in.filesize / 32;      //目录条数=文件大小/32
			int block_count = in.filesize / 512;
			Directory *dict = new Directory[dict_count];
			int j = 0;
			for (j; j < block_count; j++)
			{
				iofile.seekg(in.occupied_block[j]);
				iofile.read((char*)&dict[j * 16], 512);
			}
			if (in.filesize - block_count * 512)
			{
				iofile.seekg(in.occupied_block[j]);
				iofile.read((char*)&dict[j * 16], in.filesize - block_count * 512);
			}                                                 //目录加载完成
			char inf[50] = "/";
			cout << diclevel << "级目录（&文件）:" << endl;
			for (int m = 0; m < dict_count; m++)
			{
				strcat(inf, dict[m].filename);                 //在目录名前加“/”
				QElem Qinode;
				Qinode.inode_addr = dict[m].inode;
				strcpy(Qinode.filename, p.filename);
				strcat(Qinode.filename, inf);
				cout << Qinode.filename << endl;
				inode_queue.insert(Qinode);
			}
			cout << endl;
			diclevel++;
		}
	}
}