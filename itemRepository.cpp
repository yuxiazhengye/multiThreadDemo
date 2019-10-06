#include "itemRepository.h"
#include "vector"

void ItemRepository::produce(int item)
{
	std::unique_lock<std::mutex> lck(proMtx);
	while ((writePos + 1) % BUFF_SIZE == readPos)
	{
		{
			std::unique_lock<std::mutex> lck2(wrMtx);
			std::cout << "缓冲区满，等待缓冲区不满\n";
		}
		repo_not_full.wait(lck);
	}
	itemBuffer[writePos] = item;
	writePos++;
	if (writePos == BUFF_SIZE) writePos = 0;
	lck.unlock();
	repo_not_empty.notify_all();
}

int ItemRepository::consumer()
{
	std::unique_lock<std::mutex> lck(proMtx);
	while (writePos == readPos)
	{
		{
			std::unique_lock<std::mutex> lck2(wrMtx);
			std::cout << "缓冲区空，等待生产者生产产品\n";
		}
		repo_not_empty.wait(lck);
	}
	int item = itemBuffer[readPos];
	readPos++;
	if (readPos == BUFF_SIZE) readPos = 0;
	lck.unlock();
	repo_not_full.notify_all();
	return item;
}

void ItemRepository::produceTask()
{
	for (int i = 0; i < ITEM_SIZE; i++)
	{
		produce(i + 1);
		{
			std::lock_guard<std::mutex> lck(wrMtx);
			std::cout << "product: " << i + 1 << std::endl;
		}
	}
}

void ItemRepository::consumerTask()
{
	static int cnt = 0;
	while (1)
	{
		std::chrono::milliseconds dura(1000);
		std::this_thread::sleep_for(dura);

		int item = consumer();
		{
			std::lock_guard<std::mutex> lck(wrMtx);
			std::cout << "consumer: " << item << std::endl;
		}
		if (++cnt == ITEM_SIZE) break;
	}
}

void ItemRepository::multiProduceTask()
{
	bool ready_to_exit = false;
	while (1)
	{
		std::unique_lock<std::mutex> lck(producer_mutex);
		if (producer_item_cnt < ITEM_SIZE)
		{
			++producer_item_cnt;
			produce(producer_item_cnt);
			{
				std::lock_guard<std::mutex> lck2(wrMtx);
				std::cout << "生产者：" << std::this_thread::get_id();
				std::cout << " 正在生产第 " << producer_item_cnt << " 个产品" << std::endl;
			}
		}
		else
		{
			ready_to_exit = true;
		}
		lck.unlock();

		std::chrono::milliseconds dura(1000);
		std::this_thread::sleep_for(dura);
		if (ready_to_exit) break;
	}
	{
		std::lock_guard<std::mutex> lck2(wrMtx);
		std::cout << "生产者：" << std::this_thread::get_id()  << " 退出" << std::endl;
	}
}

void ItemRepository::multiConsumerTask()
{ 
	bool ready_to_exit = false;
	while(1)
	{
		std::unique_lock<std::mutex> lck(consumer_mutex);
		if (consumer_item_cnt < ITEM_SIZE)
		{
			int item = consumer();
			++consumer_item_cnt;
			{
				std::lock_guard<std::mutex> lck2(wrMtx);
				std::cout << "消费者：" << std::this_thread::get_id();
				std::cout << " 正在消费第 " << item << " 个产品" << std::endl;
			}
		}
		else
		{
			ready_to_exit = true;
		}
		lck.unlock();

		std::chrono::milliseconds durata(1000);
		std::this_thread::sleep_for(durata);
		if (ready_to_exit) break;
	}
	{
		std::lock_guard<std::mutex> lck2(wrMtx);
		std::cout << "消费者：" << std::this_thread::get_id() << " 退出" << std::endl;
	}
}

void ItemRepository::startMulti()
{
	std::vector<std::thread> threads;
	for (int i = 0; i < 4; i++)
	{
		threads.push_back(std::thread(&ItemRepository::multiProduceTask, this));
	}
	for (int i = 0; i < 4; i++)
	{
		threads.push_back(std::thread(&ItemRepository::multiConsumerTask, this));
	}
	for (auto& it : threads)
	{
		it.join();
	}
}

void ItemRepository::start()
{
	std::thread producers(&ItemRepository::produceTask, this);
	std::thread consumers(&ItemRepository::consumerTask, this);
	producers.join();
	consumers.join();
}

void oneProduceOneConsumer()
{
	ItemRepository item;
	item.start();
}

void multiProduceMultiConsumer()
{
	ItemRepository item;
	item.startMulti();
}