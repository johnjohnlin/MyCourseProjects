#include "panorama.h"

int main()
{
	Panorama p;
	p.add_file("1.pgm");
	p.add_file("2.pgm");
	p.add_file("3.pgm");
	p.run();
	return 0;
}
