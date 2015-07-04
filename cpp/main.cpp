#include <memory>

template<class Prototype> class Function;

template<class Ret, class ... Args>
class Function<Ret(Args...)> {
public:
  template<class Target>
  Function(Target target) : target_ptr_(std::make_shared<TargetImpl<Target>>(std::move(target))) {
  }

  Ret operator()(Args&&... args) {
    return target_ptr_->invoke(std::forward<Args>(args)...);
  }

private:
  struct TargetWrapper {
    virtual ~TargetWrapper() = default;
    virtual Ret invoke(Args&&... args) = 0;
  }; // struct TargetWrapper

  template<class Target>
  struct TargetImpl : public TargetWrapper {
    TargetImpl(Target target) : target_(std::move(target)) {
    }

    Ret invoke(Args&&... args) override {
      return target_(std::forward<Args>(args)...);
    }

    Target target_;
  }; // struct TargetImpl

  std::shared_ptr<TargetWrapper> target_ptr_;
}; // class Function

#include <iostream>
#include <string>

struct Foo {
  int operator()(const std::string &arg, int n) {
    std::cout << arg << ":" << n << std::endl;
    return static_cast<int>(arg.size());
  }
};

std::string bar(int n) {
  std::string orz("orz"), ret(std::to_string(n));
  ret.push_back(':');
  while (n > 0) {
    ret += orz;
    --n;
  }
  return std::move(ret);
}

int main() {
  Function<int(const std::string&, int)> f = Foo();
  Function<decltype(bar)> b(bar);
  std::cout << b(f("HTC", 2498)) << std::endl;

  Function<int(int)> sum = [&] (int n) -> int {
    if (n > 0) {
      return n + sum(n - 1);
    } else {
      return 0;
    }
  };
  std::cout << sum(10) << std::endl;

  return 0;
}
