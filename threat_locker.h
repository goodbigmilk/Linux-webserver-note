#ifndef PTHREAD_LOCKER_H
#define PTHREAD_LOCKER_H

// 三种线程同步机制的封装

#include <semaphore.h>
#include <pthread.h>
#include <exception>

// 封装信号量的类
class sem
{
public:
	sem()
	{
		// 初始化m_sem
		if (sem_init(&m_sem, 0, 0) != 0)
		{
			throw std::exception();
		}
	}
	~sem()
	{
		sem_destroy(&m_sem);
	}
	bool wait()
	{
		return sem_wait(&m_sem) == 0;
	}
	bool post()
	{
		return sem_post(&m_sem) == 0;
	}

private:
	sem_t m_sem;
};

// 封装互斥锁类
class locker
{
private:
	pthread_mutex_t m_mutex;
public:
	locker()
	{
		if (pthread_mutex_init(&m_mutex, nullptr) != 0)
		{
			throw std::exception();
		}
	}
	~locker()
	{
		pthread_mutex_destroy(&m_mutex);
	}
	bool lock()
	{
		return pthread_mutex_lock(&m_mutex) == 0;
	}
	bool unlock()
	{
		return pthread_mutex_unlock(&m_mutex) == 0;
	}
};

// 封装条件变量的类
class cond
{
private:
	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;
public:
	cond()
	{
		if (pthread_mutex_init(&m_mutex, nullptr) != 0)
		{
			throw std::exception();
		}
		if (pthread_cond_init(&m_cond, nullptr) != 0)
		{
			pthread_mutex_destroy(&m_mutex);
			throw std::exception();
		}
	}
	~cond()
	{
		pthread_mutex_destroy(&m_mutex);
		pthread_cond_destroy(&m_cond);
	}
	bool wait()
	{
		int ret = 0;
		pthread_mutex_lock(&m_mutex);
		// pthread_cond_wait 在等待条件变量m_cond时会自动释放掉锁m_mutex,在被唤醒会重新获得锁
		ret = pthread_cond_wait(&m_cond, &m_mutex);
		pthread_mutex_unlock(&m_mutex);
		return ret == 0;
	}
	// 唤醒等待条件变量的线程
	bool signal()
	{
		return pthread_cond_signal(&m_cond) == 0;
	}
};

#endif //PTHREAD_LOCKER_H