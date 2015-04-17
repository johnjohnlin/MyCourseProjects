#include <string>
#include "image.hpp"
#include "invbox.hpp"

int main(int argc, char* argv[])
{
  std::string name = argv[1];
  Image im;
  float *img = im.read_bmp((name + ".bmp").c_str(), true);
  im.write_bmp((name + "_original.bmp").c_str(), img);

	Param p;
	p.w = im.w;
	p.h = im.h;
	p.r = 3;
	p.mu = 10;
	p.padding = 160;
	InvBox ib(p);

  ib.solve(img);
  im.write_bmp((name + "_out.bmp").c_str(), img);

	return 0;
}
