#include <tuple>
#include <utility>

template<class ... Iterators>
struct ReturnValue {
  ReturnValue(bool res, Iterators... itrs) : result(res), iterators(std::make_tuple(itrs...)) {
  }

  bool result;
  std::tuple<Iterators...> iterators;
};

template<class Type>
ReturnValue<> find_first_common_impl(const Type &val) {
  return ReturnValue<>(true);
}

template<class Type, class Iterator, class ... Iterators>
ReturnValue<Iterator, Iterators...> find_first_common_impl(const Type &val, std::pair<Iterator, Iterator> head, std::pair<Iterators, Iterators> ...tail) {
  ReturnValue<Iterator, Iterators...> result(false, head.second, tail.second...);
  while (head.first != head.second) {
    if (*(head.first) == val) {
      auto ret = find_first_common_impl(val, tail...);
      if (ret.result) {
        result.result = true;
        result.iterators = std::tuple_cat(std::make_tuple(head.first), ret.iterators);
      }
      break;
    }
    ++(head.first);
  }
  return result;
}

template<class Iterator, class ... Iterators>
std::tuple<Iterator, Iterators...> find_first_common(std::pair<Iterator, Iterator> head, std::pair<Iterators, Iterators> ...tail) {
  std::tuple<Iterator, Iterators...> result = std::make_tuple(head.second, tail.second...);
  while (head.first != head.second) {
    auto ret = find_first_common_impl(*(head.first), tail...);
    if (ret.result) {
      result = std::tuple_cat(std::make_tuple(head.first), ret.iterators);
      break;
    }
    ++(head.first);
  }
  return result;
}

#include <deque>
#include <iostream>
#include <iterator>
#include <list>
#include <vector>

int main() {
  std::deque<int> d = {
    3, 2498, 5566
  };
  std::list<int> l = {
    10, 100, 1000, 3
  };
  std::vector<int> v = {
    0, 1, 2, 3, 4, 5
  };

  auto itrs = find_first_common(std::make_pair(v.begin(), v.end()), std::make_pair(d.begin(), d.end()), std::make_pair(l.begin(), l.end()));
  std::cout << "v:" << std::endl;
  std::copy(std::get<0>(itrs), v.end(), std::ostream_iterator<int>(std::cout, ", "));
  std::cout << std::endl;

  std::cout << "d:" << std::endl;
  std::copy(std::get<1>(itrs), d.end(), std::ostream_iterator<int>(std::cout, ", "));
  std::cout << std::endl;

  std::cout << "l:" << std::endl;
  std::copy(std::get<2>(itrs), l.end(), std::ostream_iterator<int>(std::cout, ", "));
  std::cout << std::endl;

  return 0;
}
