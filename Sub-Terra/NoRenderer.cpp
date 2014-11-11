#include "NoRenderer.h"
#include <iostream>

void NoRenderer::Destroy() {
	std::cout << "Press enter to continue.";
	std::cin.ignore(1);
}
