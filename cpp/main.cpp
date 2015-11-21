#include <cstring>

#include <chrono>
#include <iostream>
#include <string>
#include <type_traits>

namespace dc {
template<class Type, size_t N = sizeof(Type)>
struct Mask {
  static constexpr Type value = (Mask<Type, N - 1>::value << 8) | 0x80;
};

template<class Type>
struct Mask<Type, 0> {
  static constexpr Type value = 0;
};

template<class Type, typename std::enable_if<std::is_same<typename std::decay<Type>::type, char *>::value || std::is_same<typename std::decay<Type>::type, const char *>::value, int>::type = 0>
int strlen(Type &&s) {
  typedef unsigned int align_type;
  constexpr auto align = sizeof(align_type);
  constexpr auto align_mask = align - 1;

  // align
  auto char_ptr = s;
  while ((reinterpret_cast<decltype(align_mask)>(char_ptr) & align_mask) != 0) {
    if (*char_ptr == '\0') {
      return (char_ptr - s);
    }
    ++char_ptr;
  }

  // compare multiple bytes at one time to utilize cache
  auto aligned_ptr = reinterpret_cast<const align_type *>(char_ptr);
  for (;;) {
    if (((Mask<align_type>::value - *aligned_ptr) & Mask<align_type>::value) != 0) {
      // one of these bytes is '\0'
      char_ptr = reinterpret_cast<const char *>(aligned_ptr);
      for (size_t i = 0; i < align; ++i) {
        if (*char_ptr == '\0') {
          return (char_ptr - s);
        }
        ++char_ptr;
      }
    }
    ++aligned_ptr;
  }

  return -1;
}

template<int N>
constexpr int strlen(const char (&)[N]) {
  return (N - 1);
}
}

int main() {
  constexpr int iteration = 10000000;
  std::string hello_world("Hello, World!");
  volatile int len;

  std::cout << "strlen(\"Hello, World!\")... ";
  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < iteration; ++i) {
    len = strlen("Hello, World!");
  }
  auto stop = std::chrono::high_resolution_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " ms" << std::endl;

  std::cout << "strlen(hello_world)... ";
  start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < iteration; ++i) {
    len = strlen(hello_world.c_str());
  }
  stop = std::chrono::high_resolution_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " ms" << std::endl;

  std::cout << "dc::strlen(\"Hello, World!\")... ";
  start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < iteration; ++i) {
    len = dc::strlen("Hello, World!");
  }
  stop = std::chrono::high_resolution_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " ms" << std::endl;

  std::cout << "dc::strlen(hello_world)... ";
  start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < iteration; ++i) {
    len = dc::strlen(hello_world.c_str());
  }
  stop = std::chrono::high_resolution_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " ms" << std::endl;

  return 0;
}
