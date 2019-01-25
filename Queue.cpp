#include"Queue.h"
using namespace std;

Queue::Queue()
{
	head = 0;
	end = 0;
}

int Queue::insert(QElem Q)
{
	if ((end + 1) % 50 == head)
		return ERROR;
	Qqueue[end] = Q;
	end = (end + 1) % 50;
	return OK;
}

int Queue::fetch(QElem &Q)
{
	if (head == end)        //╤сап©у
		return ERROR;
	Q = Qqueue[head];
	head = (head + 1) % 50;
	return OK;
}