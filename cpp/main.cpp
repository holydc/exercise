// TMP library
/**
 * The container holding the number sequence
 */
template<int... N> struct Sequence {};

template<class, int> struct Append;

/**
 * An util class appending a number to the specified sequence
 */
template<int... BEGIN, int END>
struct Append<Sequence<BEGIN...>, END> {
  typedef Sequence<BEGIN..., END> result_type;
};

/**
 * An util class generating a number sequence in [<tt>BEGIN</tt>, <tt>END</tt>]
 */
template<int BEGIN, int END>
struct SequenceGenerator {
  static_assert(BEGIN <= END, "SequenceGenerator: invalid range");
  typedef typename Append<typename SequenceGenerator<BEGIN, END - 1>::result_type, END>::result_type result_type;
};

/**
 * Specialization of <em>SequenceGenerator</em>, the terminating condition
 */
template<int BEGIN>
struct SequenceGenerator<BEGIN, BEGIN> {
  typedef Sequence<BEGIN> result_type;
};

/**
 * An util class generating a number sequence in [0, <tt>N</tt>)
 */
template<int N>
struct IndexGenerator {
  typedef typename SequenceGenerator<0, N - 1>::result_type result_type;
};

template<template<int> class, class> struct Values;

/**
 * The container holding the calculated values
 *
 * @tparam Calculator The specified calculating method
 * @tparam ValueType The data type of the calculated values
 * @tparam N The number sequence to be calculated
 */
template<template<int> class Calculator, int... N>
struct Values<Calculator, Sequence<N...>> {
  /**
   * The array of the calculated values
   */
  static const int values[sizeof...(N)];
};

template<template<int> class Calculator, int... N>
const int Values<Calculator, Sequence<N...>>::values[sizeof...(N)] = {
  Calculator<N>::value...
};

// Example
#include <iostream>

template<int N>
struct Fibonacci {
  static_assert(N >= 0, "Fibonacci: invalid argument");
  static constexpr int value = Fibonacci<N - 2>::value + Fibonacci<N - 1>::value;
};

template<>
struct Fibonacci<1> {
  static constexpr int value = 1;
};

template<>
struct Fibonacci<0> {
  static constexpr int value = 0;
};

template<int N>
using FibonacciSequence = Values<Fibonacci, typename IndexGenerator<N>::result_type>;

int main() {
  constexpr int N = 20;
  for (int i = 0; i < N; ++i) {
    std::cout << FibonacciSequence<N>::values[i] << ", ";
  }
  std::cout << std::endl;
  return 0;
}
