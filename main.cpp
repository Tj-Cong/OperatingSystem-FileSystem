#include"FileSystem.h"
#include<iomanip>
using namespace std;

void print_cmdshell()
{
	cout << "/*********************************************/" << endl;
	cout << "���ܺţ�1 ������ʽ���ļ���" << endl;
	cout << "���ܺţ�2 ������Ŀ¼" << endl;
	cout << "���ܺţ�3 �����½��ļ�" << endl;
	cout << "���ܺţ�4 �������ļ�" << endl;
	cout << "���ܺţ�5 �����ر��ļ�" << endl;
	cout << "���ܺţ�6 �������ļ�" << endl;
	cout << "���ܺţ�7 ����д�ļ�" << endl;
	cout << "���ܺţ�8 ������λ�ļ���дָ��" << endl;
	cout << "���ܺţ�9 ����ɾ���ļ�" << endl;
	cout << "���ܺţ�0 �����˳� " << endl;
	cout << "/*********************************************/" << endl;
}

void cmdshell(FileSystem &fs)
{
	int funcnum = 1;
	while (funcnum)
	{
		cout << "��ѡ����Ӧ���ܺţ�";
		cin >> funcnum;
		switch (funcnum)
		{
		case 1: {
			fs.fformat();
			cout << "��ʽ����ɣ�" << endl;
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
			cout << "�������ļ�����Ŀ¼֮���ԡ�/��������" << endl;
			cin >> filename;
			if (fs.fcreat(filename) != ERROR)
				cout << "�����ɹ���" << endl;
			cout << endl;
			break;
		}
		case 4: {
			char filename[50];
			int mode;
			cout << "�������ļ�����Ŀ¼֮���ԡ�/��������" << endl;
			cin >> filename;
			cout << "�������ļ��򿪷�ʽ��1-ֻ����2-ֻд��3-�ɶ���д" << endl;
			cin >> mode;
			if (fs.fopen(filename, mode) != -1)
				cout << "�򿪳ɹ����ļ���Ϊ��" << fs.fopen(filename, mode) << endl;
			cout << endl;
			break;
		}
		case 5: {
			int fd;
			cout << "�������ļ��ţ�" << endl;
			cin >> fd;
			fs.fclose(fd);
			cout << "�ļ��ѹرգ�" << endl;
			cout << endl;
			break;
		}
		case 6: {
			int fd, length;
			cout << "�����������ļ��źͶ�ȡ���ȣ�" << endl;
			cin >> fd >> length;
			char *buffer = new char[length+1];
			if (fs.fread(fd, buffer, length) == OK)
			{
				cout << "��������Ϊ��" << endl;
				cout << buffer << endl;
			}
			cout << endl;
			break;
		}
		case 7: {
			int fd;
			
			cout << "ע��Ϊ���ڲ��ԣ�д��������ϵͳָ��\n��д��800�ֽڣ�����500-520�ֽ�ȫΪ��2���������ֽ�ȫΪ��1����" << endl;
			cout << "�������ļ��ţ�" << endl;
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
			cout << "�����������ļ��źͶ�дָ��λ�ã���λ���ֽڣ���" << endl;
			cin >> fd >> position;
			fs.flseek(fd, position);
			cout << endl;
			break;
		}
		case 9: {
			char filename[50];
			cout << "�������ļ�����Ŀ¼֮���ԡ�/��������" << endl;
			cin >> filename;
			if (fs.fdelete(filename) == OK)
				cout << "��ɾ����" << endl;
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