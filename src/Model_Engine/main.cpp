#include "ModelEngine.h"

int main()
{
  ModelEngine model;

  try
  {

    for (int times = 0; times < 6000; ++times)
    {
      auto ms = model.run();
      if (ms.valid_state == false)
      {
        std::cerr << "Error";
        break;
      }
    }

  }
  catch (const std::exception& ex)
  {
    std::cerr << ex.what();
  }

  std::cin.ignore();

  return 0;
}