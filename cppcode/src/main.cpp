#include "thread_test.h"
#include <future>

int main(int argc, char **argv)
{
    std::future<int> ans = std::async(std::launch::deferred, []() -> int
                                      { return 6; });

    ans.wait(); // wait配合deferred是在本线程实现的异步 没有创建线程
    std::future_status ret = ans.wait_for(std::chrono::seconds(0));
    // std::cout << ans.get() << std::endl; // 只能调用一次
    // 多次调用方法如下
    /*
    std::shared_future<int> val(std::move(ans));
    std::cout << val.get() << std::endl;
    std::cout << val.get() << std::endl;
    std::cout << val.get() << std::endl;
    */
    if (ret == std::future_status::deferred)
        std::cout << "异步未执行\n";
    else if (ret == std::future_status::ready)
        std::cout << "异步执行\n";
    if (ret == std::future_status::timeout)
        std::cout << "线程创建了\n";
    else
        std::cout << "线程没有创建\n";

    return 0;
}