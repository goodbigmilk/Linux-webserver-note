#include <time.h>
#include <stddef.h>
#include <cstdio>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

// �û����ݽṹ �ͻ��˵�socket��ַ socket�ļ������� ������ ��ʱ��
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer* timer;
};

class util_timer
{
public:
    util_timer() :prev(nullptr), next(nullptr) {};
public:
    util_timer* prev; // ǰ��
    util_timer* next; // ���

    time_t expire; // ��ʱʱ��
    client_data* user_data; // �ص���������Ŀͻ�������
    void (*cb_func)(client_data*); // �ص�����
};

// ��ʱ������ ���� ˫�� ����ͷβ�ڵ�
class sort_timer_lst
{
public:
    sort_timer_lst() :head(nullptr), tail(nullptr) {};
    // ����ʱɾ���������еĶ�ʱ��
    ~sort_timer_lst()
    {
        util_timer* temp = head;
        while (nullptr != temp)
        {
            delete head;
            head = temp->next;
            temp = head;
        }
    }
    // ��Ŀ�궨ʱ����ӵ�������
    void add_timer(util_timer* timer)
    {
        if (nullptr == timer)
        {
            return;
        }
        if (nullptr == head)
        {
            head = timer;
            tail = timer;
            return;
        }
        // ��ʱʱ��С����������Сʱ�� ֱ�Ӳ���ͷ��
        if (timer->expire < head->expire)
        {
            head = timer;
            timer->next = tail;
            tail->prev = timer;
            return;
        }
        // ��Ҫ���뵽������ ������β��
        add_timer(timer, head);
    }

    // ����timer��ʱ��expire
    void adjust_timer(util_timer* timer)
    {
        if (nullptr == timer)
        {
            return;
        }
        util_timer* temp = timer->next;
        if (timer == tail || (timer->expire < temp->expire))
        {
            return;
        }
        // ��������Ķ�ʱ�� ��head ����Ҫȡ�������ʱ��
        if (timer == head)
        {
            head = head->next;
            head->prev = nullptr;
            timer->next = nullptr;
            add_timer(timer, head);
        }
        else
        {
            // ����head ���м䲿��
            timer->prev->next = timer->next;
            timer->next->prev = timer->prev;
            add_timer(timer, timer->next);
        }
    }

    void del_timer(util_timer* timer)
    {
        if (nullptr == timer)
        {
            return;
        }
        if ((timer == head) && (timer == tail))
        {
            delete timer;
            head = nullptr;
            tail = nullptr;
            return;
        }
        if (timer == head)
        {
            head = head->next;
            head->prev = nullptr;
            delete timer;
            return;
        }
        if (timer == tail)
        {
            tail = tail->prev;
            tail->next = nullptr;
            delete timer;
            return;
        }
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        delete timer;
    }

    void tick()
    {
        if (nullptr == head)
        {
            return;
        }
        printf("timer tick\n");
        time_t cur = time(NULL);
        util_timer* temp = head;
        while (nullptr != head)
        {
            if (cur < temp->expire)
            {
                break;
            }
            temp->cb_func(temp->user_data);
            head = temp->next;
            if (nullptr != head)
            {
                head->prev = nullptr;
            }
            delete temp;
            temp = head;
        }
    }
private:
    util_timer* head;
    util_timer* tail;

private:
    void add_timer(util_timer* timer, util_timer* lst_head)
    {
        util_timer* temp = lst_head;
        // ����temp�ڵ�Ϊtimer->expire < temp->expire
        while (timer->expire > temp->expire)
        {
            if (temp->next != nullptr)
            {
                temp = temp->next;
            }
            else
            {
                // ���temp == tail ����expire��Ȼ�� ��˵��Ӧ�ò���β��
                tail->next = timer;
                timer->prev = tail;
                tail = timer;
                return;
            }
        }

        timer->next = temp;
        timer->prev = temp->prev;
        temp->prev->next = timer;
        temp->prev = timer;
    }
};