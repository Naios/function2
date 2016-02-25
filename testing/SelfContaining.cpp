
#include "function.hpp"

bool testSelfContaining()
{
  fu2::function<bool()> first = []
  {
    return true;
  };
  fu2::unique_function<bool()> second = first;
  return first() && second();
}
