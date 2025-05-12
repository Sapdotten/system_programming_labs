#include <iostream>
#include <vector>
#include <thread>
#include <barrier>
#include <cstdlib>
#include <ctime>

constexpr int num_threads = 4;
std::barrier sync_point(num_threads);

void fill_random(int *arr, const size_t size, int min_value, int max_value)
{
    std::srand(std::time(nullptr));
    for (int i = 0; i < size; ++i)
    {
        arr[i] = min_value + std::rand() % (max_value - min_value + 1);
    }
}

void search_task(int id, const int *data, int start, int end, int target,
                 std::vector<std::vector<int>> &results,
                 std::barrier<> &sync_point)
{
    for (int i = start; i < end; ++i)
        if (data[i] == target)
            results[id].push_back(i);

    sync_point.arrive_and_wait(); // синхронизируем завершение поиска
}

int main()
{
    const size_t size = 100000;
    int data[size];
    fill_random(data, size, 0, 100);
    int target = 5;

    const int num_threads = 10;
    int chunk_size = size / num_threads;

    std::vector<std::vector<int>> results(num_threads);
    std::barrier sync_point(num_threads);

    std::thread threads[num_threads];

    for (int i = 0; i < num_threads; ++i)
    {
        int start = i * chunk_size;
        int end = (i == num_threads - 1) ? size : (i + 1) * chunk_size;
        threads[i] = std::thread(search_task, i, data, start, end, target,
                                 std::ref(results), std::ref(sync_point));
    }

    for (auto &t : threads)
        t.join();

    // теперь выводим в главном потоке
    for (const auto &thread_result : results)
    {
        for (int idx : thread_result)
            std::cout << idx << "\t";
        std::cout << std::endl
                  << std::endl;
    }
    return 0;
}
