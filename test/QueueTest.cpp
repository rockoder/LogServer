#include "Queue.h"
#include <string>
#include <boost/lexical_cast.hpp>
#include <iostream>

using std::shared_ptr;
using std::unique_ptr;
using std::mutex;
using std::make_shared;
using std::move;

using namespace std;

Queue<string> q;

void fillQueue(string temp, size_t total)
{
	size_t i = 0;
	string str;

	while (i != total)
	{
		str = temp + boost::lexical_cast<string>(i);
		q.push(str);
		++i;
	}
}

void printQueue(size_t total)
{
	size_t i = 0;
	string str;

	while (i != total)
	{
		q.waitAndPop(str);

		cout << str << endl;
		++i;
	}
}

int main()
{
	string s1 = "t1 string ";
	string s2 = "t2 string ";

	size_t size1 = 1000;
	size_t size2 = 500;

	thread t1(fillQueue, s1, size1);
	thread t2(fillQueue, s2, size2);

	thread t3(printQueue, size1 + size2);

	t1.join();
	t2.join();
	t3.join();

	assert(q.empty());

	return 0;
}