#include"FileSystem.h"
#include<iomanip>
using namespace std;

void print_cmdshell()
{
	cout << "/*********************************************/" << endl;
	cout << "功能号：1 ——格式化文件卷" << endl;
	cout << "功能号：2 ——列目录" << endl;
	cout << "功能号：3 ——新建文件" << endl;
	cout << "功能号：4 ——打开文件" << endl;
	cout << "功能号：5 ——关闭文件" << endl;
	cout << "功能号：6 ——读文件" << endl;
	cout << "功能号：7 ——写文件" << endl;
	cout << "功能号：8 ——定位文件读写指针" << endl;
	cout << "功能号：9 ——删除文件" << endl;
	cout << "功能号：0 ——退出 " << endl;
	cout << "/*********************************************/" << endl;
}

void cmdshell(FileSystem &fs)
{
	int funcnum = 1;
	while (funcnum)
	{
		cout << "请选择相应功能号：";
		cin >> funcnum;
		switch (funcnum)
		{
		case 1: {
			fs.fformat();
			cout << "格式化完成！" << endl;
			cout << endl;
			break;
		}
		case 2: {
			fs.ls();
			cout << endl;
			break;
		}
		case 3: {
			char filename[50];
			cout << "请输入文件名，目录之间以‘/’隔开：" << endl;
			cin >> filename;
			if (fs.fcreat(filename) != ERROR)
				cout << "创建成功！" << endl;
			cout << endl;
			break;
		}
		case 4: {
			char filename[50];
			int mode;
			cout << "请输入文件名，目录之间以‘/’隔开：" << endl;
			cin >> filename;
			cout << "请输入文件打开方式：1-只读；2-只写；3-可读可写" << endl;
			cin >> mode;
			if (fs.fopen(filename, mode) != -1)
				cout << "打开成功！文件号为：" << fs.fopen(filename, mode) << endl;
			cout << endl;
			break;
		}
		case 5: {
			int fd;
			cout << "请输入文件号：" << endl;
			cin >> fd;
			fs.fclose(fd);
			cout << "文件已关闭！" << endl;
			cout << endl;
			break;
		}
		case 6: {
			int fd, length;
			cout << "请依次输入文件号和读取长度：" << endl;
			cin >> fd >> length;
			char *buffer = new char[length+1];
			if (fs.fread(fd, buffer, length) == OK)
			{
				cout << "读出内容为：" << endl;
				cout << buffer << endl;
			}
			cout << endl;
			break;
		}
		case 7: {
			int fd;
			
			cout << "注：为便于测试，写入内容有系统指定\n（写入800字节，其中500-520字节全为‘2’，其余字节全为‘1’）" << endl;
			cout << "请输入文件号：" << endl;
			cin >> fd;
			
			char buffer[800];
			for (int i = 0; i < 800; i++)
			{
				if (i >= 500 && i <= 520)
					buffer[i] = '2';
				else
					buffer[i] = '1';
			}
			fs.fwrite(fd, (char*)&buffer[0], 800);
			cout << endl;
			break;
		}
		case 8: {
			int fd, position;
			cout << "请依次输入文件号和读写指针位置（单位：字节）：" << endl;
			cin >> fd >> position;
			fs.flseek(fd, position);
			cout << endl;
			break;
		}
		case 9: {
			char filename[50];
			cout << "请输入文件名，目录之间以‘/’隔开：" << endl;
			cin >> filename;
			if (fs.fdelete(filename) == OK)
				cout << "已删除！" << endl;
			cout << endl;
			break;
		}
		case 0:break;
		default:
			break;
		}
	}
}

int main()
{
	FileSystem Fs("myDisk.txt");
	//Fs.fformat();
	Fs.loadFAT();
	Fs.loadsuperblock();
	print_cmdshell();
	cmdshell(Fs);
	/*char path[] = "root/usr/jerry\0";
	int addr = Fs.fcreat(path);
	cout <<"jerry's inode_addr: "<< addr << endl;
	int fd = Fs.fopen(path, 3);
	cout << "jerry's inode_addr: " << addr << endl;
	char buffer[800];
	char readout[21];
	for (int i = 0; i < 800; i++)
	{
		if(i>=500&&i<=520)
			buffer[i] = '2';
		else
			buffer[i] = '1';
	}
		
	int f = Fs.fwrite(fd, (char*)&buffer[0], 800);
	inode i;
	Fs.iofile.seekg(Fs.file[fd]->inode_addr);
	Fs.iofile.read((char*)&i, sizeof(i));
	int ff = Fs.flseek(fd, 500);
	int fff = Fs.fread(fd, (char*)&readout, 20);
	cout << readout;
	Fs.ls();*/
	return 0;
}