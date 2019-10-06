#pragma once
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

const static int BUFF_SIZE = 10;
const static int ITEM_SIZE = 50;

struct ItemRepository{
	ItemRepository() :readPos(0), writePos(0),producer_item_cnt(0), consumer_item_cnt(0)
	{
		memset(itemBuffer, 0, sizeof(int)*BUFF_SIZE);
	}
	void produceTask();
	void consumerTask();
	void multiProduceTask();
	void multiConsumerTask();
	void start();
	void startMulti();
private:
	void produce(int item);
	int consumer();
private:
	size_t readPos;
	size_t writePos;
	int producer_item_cnt;
	int consumer_item_cnt;
	int itemBuffer[BUFF_SIZE];
	std::mutex proMtx;
	std::mutex wrMtx;
	std::mutex producer_mutex;
	std::mutex consumer_mutex;
	std::condition_variable repo_not_full;
	std::condition_variable repo_not_empty;
};

void oneProduceOneConsumer(void);
void multiProduceMultiConsumer(void);
