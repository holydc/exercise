void create_thread(void(*thread_proc)(void*), void *arg) {
  thread_proc(arg);
}

template<int... N>
struct Sequence {
};

template<class, int> struct Append;

template<int... BEGIN, int END>
struct Append<Sequence<BEGIN...>, END> {
  typedef Sequence<BEGIN..., END> result_type;
};

template<int BEGIN, int END>
struct SequenceGenerator {
  typedef typename Append<typename SequenceGenerator<BEGIN, END - 1>::result_type, END>::result_type result_type;
};

template<int BEGIN>
struct SequenceGenerator<BEGIN, BEGIN> {
  typedef Sequence<BEGIN> result_type;
};

#include <tuple>
#include <type_traits>
#include <utility>

class Thread {
public:
  template<class Function, class... Args>
  explicit Thread(Function &&f, Args&&... args) {
    std::tuple<typename std::decay<Function>::type, typename std::decay<Args>::type...> t(
      std::forward<Function>(f),
      std::forward<Args>(args)...);
    create_thread(
      ThreadHelper<typename std::decay<Function>::type, typename std::decay<Args>::type...>::thread_proc,
      &t);
  }

private:
  template<class Function, class... Args>
  struct ThreadHelper {
    static void thread_proc(void *arg) {
      helper(arg, typename SequenceGenerator<1, sizeof...(Args)>::result_type());
    }

    template<int... N>
    static void helper(void *arg, Sequence<N...>&&) {
      auto tuple = reinterpret_cast<std::tuple<Function, Args...>*>(arg);
      std::get<0>(*tuple)(std::get<N>(*tuple)...);
    }
  };
};

#include <iostream>
#include <string>

int main() {
  Thread t([] (int a, const std::string &b, bool c) {
    std::cout << "a:" << a << " b:" << b << " c:" << std::boolalpha << c << std::endl;
  }, 5566, "Hello, World!", false);
  return 0;
}
