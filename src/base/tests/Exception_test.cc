#include "../Exception.h"

#include <functional>
#include <vector>
#include <string>
#include <stdio.h>

class Bar
{
 public:
  void test(std::vector<std::string> names = {})
  {
   /* printf("Stack:\n%s\n", ex.stackTrace());
    [] {
      printf("Stack inside lambda:\n%s\n", ex.stackTrace());
    }();*/
    std::function<void()> func([] {
      printf("Stack inside std::function:\n");
    });
    func();

    func = std::bind(&Bar::callback, this);
    func();

    throw DJX::Exception("test");
  }

 private:
   void callback()
   {
	   printf("callback\n");
   }
};

void foo()
{
  Bar b;
  b.test();
}

int main()
{
  try
  {
    foo();
  }
  catch (const DJX::Exception& ex)
  {
    printf("reason: %s\n", ex.what());
    printf("stack trace:\n%s\n", ex.stackTrace());
  }
}