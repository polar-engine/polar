#include "NoRenderer.h"
#include <iostream>

NoRenderer::NoRenderer() {

}

NoRenderer::~NoRenderer() {

}

void NoRenderer::Init() {
	std::cout << "Press enter to continue.";
	std::cin.ignore(1);
}
