#include "Polar.h"
#include <utility>

Polar::Polar(Renderer &renderer) : _renderer(renderer) {

}

Polar::~Polar() {

}

void Polar::Init() {
	_renderer.Init();
}
