#include <memory>
#include <utility>

template<class> class Function;

template<class Ret, class... Args>
class Function<Ret(Args...)> {
public:
  template<class Callable>
  Function(Callable callable) : impl_(std::make_shared<Impl<Callable>>(std::move(callable))) {
  }

  Ret operator()(Args... args) {
    return impl_->invoke(std::forward<Args>(args)...);
  }

private:
  struct ImplBase {
    virtual ~ImplBase() = default;
    virtual Ret invoke(Args... args) = 0;
  }; // struct ImplBase

  template<class Callable>
  struct Impl : public ImplBase {
    Impl(Callable callable) : callable_(std::move(callable)) {
    }

    Ret invoke(Args... args) override {
      return callable_(std::forward<Args>(args)...);
    }

    Callable callable_;
  }; // struct Impl

  template<class Method>
  struct ClassMethodImpl : public ImplBase {
    ClassMethodImpl(Method method) : method_(method) {
    }

    Ret invoke(Args... args) override {
      return bind(std::forward<Args>(args)...);
    }

    template<class Type, class... _Args>
    Ret bind(Type *_this, _Args&&... args) {
      return (_this->*method_)(std::forward<_Args>(args)...);
    }

    template<class Type, class... _Args>
    Ret bind(Type &_this, _Args&&... args) {
      return (_this.*method_)(std::forward<_Args>(args)...);
    }

    Method method_;
  }; // struct ClassMethodImpl

  template<class Type, class... _Args>
  struct Impl<Ret(Type::*)(_Args...)> : public ClassMethodImpl<Ret(Type::*)(_Args...)> {
    typedef Ret(Type::*Method)(_Args...);

    Impl(Method method) : ClassMethodImpl<Method>(std::move(method)) {
    }
  }; // struct Impl<Ret(Type::*)(_Args...)>

  template<class Type, class... _Args>
  struct Impl<Ret(Type::*)(_Args...) const> : public ClassMethodImpl<Ret(Type::*)(_Args...) const> {
    typedef Ret(Type::*Method)(_Args...) const;

    Impl(Method method) : ClassMethodImpl<Method>(std::move(method)) {
    }
  }; // struct Impl<Ret(Type::*)(_Args...) const>

  std::shared_ptr<ImplBase> impl_;
}; // class Function

#include <iostream>

struct Foo {
  Foo(int n) : member(n) {
    std::cout << "this:" << this << std::endl;
  }

  int method(int a) {
    std::cout << "method a:" << a << " this:" << this << std::endl;
    return member;
  }

  int const_method(const char *a) const {
    std::cout << "const_method a:" << a << " this:" << this << std::endl;
    return member;
  }

  int member;
};

void test_method() {
  std::cout << "enter " << __func__ << std::endl;
  Function<int(Foo &, int)> f(&Foo::method);
  Foo foo(5566);
  std::cout << f(foo, 2498) << std::endl;
  std::cout << "leave " << __func__ << std::endl;
}

void test_const_method() {
  std::cout << "enter " << __func__ << std::endl;
  Function<int(Foo *, const char *)> f(&Foo::const_method);
  Foo foo(5566);
  std::cout << f(&foo, "Hello, World!") << std::endl;
  std::cout << "leave " << __func__ << std::endl;
}

void test() {
  std::cout << "enter " << __func__ << std::endl;
  Function<void()> func(&test_const_method);
  func();

  Function<int(int)> lambda = [&lambda] (int n) -> int {
    return ((n == 1) ? 1 : (n * lambda(n - 1)));
  };
  std::cout << lambda(5) << std::endl;
  std::cout << "leave " << __func__ << std::endl;
}

int main() {
  test_method();
  //test_const_method();
  test();
  return 0;
}
