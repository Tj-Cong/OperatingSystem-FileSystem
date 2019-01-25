#pragma once
#include"FileSystem.h"
typedef struct  QElem
{
	char filename[60];
	int inode_addr;
}QElem;

class Queue
{
public:
	QElem Qqueue[50];
	int head;
	int end;
public:
	Queue();
	int insert(QElem Q);
	int fetch(QElem &Q);
};