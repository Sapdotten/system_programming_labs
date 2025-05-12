#include <pthread.h>
#include <queue>
#include <optional>
#include <iostream>
#include <unistd.h>
#define RESET "\033[0m"
#define PURPLE "\033[35m"
#define GREEN "\033[32m"

template <typename T>
class ThreadSafeQueue
{
private:
    std::queue<T> data_;
    const size_t capacity_;
    pthread_mutex_t mutex_;
    pthread_cond_t not_full_;
    pthread_cond_t not_empty_;

public:
    explicit ThreadSafeQueue(size_t capacity)
        : capacity_(capacity)
    {
        std::cout << "Queue initialized\n";
        pthread_mutex_init(&mutex_, nullptr);
        pthread_cond_init(&not_full_, nullptr);
        pthread_cond_init(&not_empty_, nullptr);
    }

    ~ThreadSafeQueue()
    {
        std::cout << "Queue destroyed\n";
        pthread_mutex_destroy(&mutex_);
        pthread_cond_destroy(&not_full_);
        pthread_cond_destroy(&not_empty_);
    }

    void push(const T &item)
    {
        pthread_mutex_lock(&mutex_);
        while (data_.size() >= capacity_)
        {
            pthread_cond_wait(&not_full_, &mutex_);
        }
        data_.push(item);
        pthread_cond_signal(&not_empty_);
        pthread_mutex_unlock(&mutex_);
    }

    T pop()
    {
        pthread_mutex_lock(&mutex_);
        while (data_.empty())
        {
            pthread_cond_wait(&not_empty_, &mutex_);
        }
        T item = data_.front();
        data_.pop();
        pthread_cond_signal(&not_full_);
        pthread_mutex_unlock(&mutex_);
        return item;
    }

    std::optional<T> try_pop()
    {
        pthread_mutex_lock(&mutex_);
        if (data_.empty())
        {
            pthread_mutex_unlock(&mutex_);
            return std::nullopt;
        }
        T item = data_.front();
        data_.pop();
        pthread_mutex_unlock(&mutex_);
        return item;
    }

    bool try_push(const T &item)
    {
        pthread_mutex_lock(&mutex_);
        if (data_.size() >= capacity_)
        {
            pthread_mutex_unlock(&mutex_);
            return false;
        }
        data_.push(item);
        pthread_mutex_unlock(&mutex_);
        return true;
    }

    bool is_empty() const
    {
        pthread_mutex_lock(&mutex_);
        bool result = data_.empty();
        pthread_mutex_unlock(&mutex_);
        return result;
    }

    bool is_full() const
    {
        pthread_mutex_lock(&mutex_);
        bool result = data_.size() >= capacity_;
        pthread_mutex_unlock(&mutex_);
        return result;
    }

    void print_contents()
    {
        pthread_mutex_lock(&mutex_);
        std::queue<T> copy = data_;
        std::cout << "Queue: ";
        while (!copy.empty())
        {
            std::cout << copy.front() << " ";
            copy.pop();
        }
        std::cout << std::endl;
        pthread_mutex_unlock(&mutex_);
    }
};

const int NUM_WRITERS = 3;
const int NUM_READERS = 2;

void *writer_thread(void *arg)
{
    static int id_counter = 0;
    int id = id_counter++;
    auto *queue = static_cast<ThreadSafeQueue<int> *>(arg);

    for (int i = 0; i < 5; ++i)
    {
        queue->push(i);
        std::cout << PURPLE << "Writer " << id << " pushed: " << i << RESET << std::endl;
        sleep(1);
    }
    return nullptr;
}

void *reader_thread(void *arg)
{
    static int id_counter = 0;
    int id = id_counter++;
    auto *queue = static_cast<ThreadSafeQueue<int> *>(arg);

    for (int i = 0; i < 5; ++i)
    {
        int value = queue->pop();
        std::cout << GREEN << "Reader " << id << " popped: " << value << RESET << std::endl;
        sleep(2);
    }
    return nullptr;
}

int main()
{
    ThreadSafeQueue<int> queue(5);

    pthread_t writers[NUM_WRITERS];
    pthread_t readers[NUM_READERS];

    for (int i = 0; i < NUM_WRITERS; ++i)
    {
        pthread_create(&writers[i], nullptr, writer_thread, &queue);
    }

    for (int i = 0; i < NUM_READERS; ++i)
    {
        pthread_create(&readers[i], nullptr, reader_thread, &queue);
    }

    for (int i = 0; i < NUM_WRITERS; ++i)
    {
        pthread_join(writers[i], nullptr);
    }

    for (int i = 0; i < NUM_READERS; ++i)
    {
        pthread_join(readers[i], nullptr);
    }

    return 0;
}
