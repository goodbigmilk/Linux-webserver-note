// 按不同顺序访问互斥锁导致死锁

#include <pthread.h>
#include <unistd.h>
#include <iostream>

int a = 0;
int b = 0;

// 声明两个互斥锁
pthread_mutex_t mutex_a;
pthread_mutex_t mutex_b;

void* another(void* arg)
{
	// 获取锁b实现对b的单独操作
	pthread_mutex_lock(&mutex_b);
	std::cout << "child thread, got b, waitting a" << std::endl;
	//sleep(5);
	++b;
	// 获取锁a，实现对a的单独操作
	pthread_mutex_lock(&mutex_a);
	b += a++;
	// 操作结束，解锁
	pthread_mutex_unlock(&mutex_a);
	pthread_mutex_unlock(&mutex_b);
}

int main()
{
	// 线程id
	pthread_t id;

	// 创建两个默认属性的锁
	pthread_mutex_init(&mutex_a, nullptr);
	pthread_mutex_init(&mutex_b, nullptr);

	// 创建子线程（线程号,属性，执行函数,函数传参）
	pthread_create(&id, nullptr, another, nullptr);

	sleep(1);
	pthread_mutex_lock(&mutex_a);
	std::cout << "parent thread, got a, waitting b" << std::endl;
	//sleep(5);
	a++;
	pthread_mutex_lock(&mutex_b);
	a += b++;
	pthread_mutex_unlock(&mutex_b);
	pthread_mutex_unlock(&mutex_a);

	// 回收子线程
	pthread_join(id, nullptr);

	// 锁用完了，销毁锁
	pthread_mutex_destroy(&mutex_a);
	pthread_mutex_destroy(&mutex_b);

	return 0;
}