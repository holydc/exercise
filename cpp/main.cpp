#include <memory>

template<class Prototype> class Function;

template<class Ret, class... Args>
class Function<Ret(Args...)> {
public:
  template<class Target>
  Function(Target target) : target_(std::make_shared<GeneralTarget<Target>>(std::move(target))) {
  }

  template<class Type>
  Function(Type *instance, Ret(Type::*method)(Args...)) : target_(std::make_shared<BindTarget<Type>>(instance, method)) {
  }

  Ret operator()(Args... args) {
    return target_->invoke(std::forward<Args>(args)...);
  }

private:
  struct TargetWrapper {
    virtual ~TargetWrapper() = default;
    virtual Ret invoke(Args... args) = 0;
  }; // struct TargetWrapper

  template<class Target>
  struct GeneralTarget : public TargetWrapper {
    GeneralTarget(Target target) : target_(std::move(target)) {
    }

    Ret invoke(Args... args) override {
      return target_(std::forward<Args>(args)...);
    }

    Target target_;
  }; // struct GeneralTarget

  template<class Type>
  struct BindTarget : public TargetWrapper {
    BindTarget(Type *instance, Ret(Type::*method)(Args...)) : instance_(instance), method_(method) {
    }

    Ret invoke(Args... args) override {
      return (instance_->*method_)(std::forward<Args>(args)...);
    }

    Type *instance_;
    Ret(Type::*method_)(Args...);
  }; // struct BindTarget

  std::shared_ptr<TargetWrapper> target_;
}; // class Function

template<class Type, class Ret, class... Args>
Function<Ret(Args...)> operator->*(std::shared_ptr<Type> instance, Ret(Type::*method)(Args...)) {
  return Function<Ret(Args...)>(instance.get(), method);
}

#include <iostream>

struct Foo {
  Foo(int n) : n_(n) {
  }

  void method() {
    std::cout << "Hello, World! " << n_ << std::endl;
    ++n_;
  }

  int n_;
};

int main() {
  std::shared_ptr<Foo> foo = std::make_shared<Foo>(5566);
  (foo->*(&Foo::method))();

  auto method = &Foo::method;
  (foo->*method)();

  auto func = foo->*method;
  func();
  return 0;
}
