// ����ͬ˳����ʻ�������������

#include <pthread.h>
#include <unistd.h>
#include <iostream>

int a = 0;
int b = 0;

// ��������������
pthread_mutex_t mutex_a;
pthread_mutex_t mutex_b;

void* another(void* arg)
{
	// ��ȡ��bʵ�ֶ�b�ĵ�������
	pthread_mutex_lock(&mutex_b);
	std::cout << "child thread, got b, waitting a" << std::endl;
	//sleep(5);
	++b;
	// ��ȡ��a��ʵ�ֶ�a�ĵ�������
	pthread_mutex_lock(&mutex_a);
	b += a++;
	// ��������������
	pthread_mutex_unlock(&mutex_a);
	pthread_mutex_unlock(&mutex_b);
}

int main()
{
	// �߳�id
	pthread_t id;

	// ��������Ĭ�����Ե���
	pthread_mutex_init(&mutex_a, nullptr);
	pthread_mutex_init(&mutex_b, nullptr);

	// �������̣߳��̺߳�,���ԣ�ִ�к���,�������Σ�
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

	// �������߳�
	pthread_join(id, nullptr);

	// �������ˣ�������
	pthread_mutex_destroy(&mutex_a);
	pthread_mutex_destroy(&mutex_b);

	return 0;
}