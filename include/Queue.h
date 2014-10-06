#ifndef _QUEUE_H
#define _QUEUE_H

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

template<typename T>
class Queue
{
private:
	struct Node
	{
		std::shared_ptr<T> data;
		std::unique_ptr<Node> next;
	};

public:
	Queue():
		head(new Node()), tail(head.get())
	{	}

	Queue(const Queue&) = delete;	// Uncopyable
	Queue& operator=(const Queue&) = delete;	// Also disable assignment

	std::shared_ptr<T> tryPop()
	{
		std::unique_ptr<Node> oldHead = tryPopHead();

		return oldHead ? oldHead->data : std::shared_ptr<T>();
	}

	bool tryPop(T& value)
	{
		std::unique_ptr<Node> const oldHead = tryPopHead(value);

		return oldHead;
	}

	std::shared_ptr<T> waitAndPop()
	{
		std::unique_ptr<Node> const oldHead = waitPopHead();

		return oldHead->data;
	}

	void waitAndPop(T& value)
	{
		std::unique_ptr<Node> const oldHead = waitPopHead(value);
	}

	bool empty()
	{
		std::lock_guard<std::mutex> headLock(headMutex);

		return head.get() == getTail();
	}

	void push(T newValue)
	{
		std::shared_ptr<T> newData(std::make_shared<T>(std::move(newValue)));

		std::unique_ptr<Node> p(new Node());

		{
			std::lock_guard<std::mutex> tailLock(tailMutex);

			tail->data = newData;

			Node* const newTail = p.get();
			tail->next = std::move(p);

			tail = newTail;
		}

		dataCond.notify_one();
	}

private:
	Node* getTail()
	{
		std::lock_guard<std::mutex> tailLock(tailMutex);
		
		return tail;
	}

	std::unique_ptr<Node> popHead()
	{
		std::unique_ptr<Node> oldHead = std::move(head);
		head = std::move(oldHead->next);
		
		return oldHead;
	}

	std::unique_lock<std::mutex> waitForData()
	{
		std::unique_lock<std::mutex> headLock(headMutex);
		dataCond.wait(headLock, [&]{ return head.get() != getTail(); });

		return std::move(headLock);
	}

	std::unique_ptr<Node> waitPopHead()
	{
		std::unique_lock<std::mutex> headLock(waitForData());
		
		return popHead();
	}

	std::unique_ptr<Node> waitPopHead(T& value)
	{
		std::unique_lock<std::mutex> headLock(waitForData());
		value = move(*head->data);

		return popHead();
	}

	std::unique_ptr<Node> tryPopHead()
	{
		std::lock_guard<std::mutex> headLock(headMutex);

		if (head.get() == getTail())
		{
			return std::unique_ptr<Node>();
		}

		return popHead();
	}

	std::unique_ptr<Node> tryPopHead(T& value)
	{
		std::lock_guard<std::mutex> headLock(headMutex);

		if (head.get() == getTail())
		{
			return std::unique_ptr<Node>();
		}
		
		value = std::move(*head->data);
		
		return popHead();
	}

private:
	std::unique_ptr<Node> head;
	std::mutex headMutex;
	
	Node* tail;
	std::mutex tailMutex;

	std::condition_variable dataCond;
};

#endif