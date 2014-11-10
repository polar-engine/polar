#include "SubTerra.h"
#include "Polar.h"
#include "NoRenderer.h"

SubTerra::SubTerra() {

}

SubTerra::~SubTerra() {

}

void SubTerra::Run(const std::vector<std::string> &&) {
	NoRenderer renderer;
	Polar engine = Polar(renderer);
	engine.Init();
}
