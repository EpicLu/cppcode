#include "thread_test.h"
#include <future>

int main(int argc, char **argv)
{
    std::future<int> ans = std::async(std::launch::async, []() -> int
                                      { return 6; });

    std::future_status ret = ans.wait_for(std::chrono::seconds(0));
    if (ret == std::future_status::timeout)
    {
        std::cout << "线程创建了\n";
    }
    else
    {
        std::cout << "线程没有创建\n";
    }
    std::cout << ans.get() << std::endl;

    return 0;
}