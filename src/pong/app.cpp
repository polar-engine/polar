#include <pong/app.h>
#include <polar/system/renderer/gl32.h>

namespace pong {
	app::app(polar::core::polar &engine) {
		using namespace polar;
		engine.addstate("root", [] (core::polar *engine, core::state &st) {
			st.addsystem_as<system::renderer::base, system::renderer::gl32>("2d");
		});
		engine.addstate("game", [] (core::polar *engine, core::state &st) {
			IDType leftPaddle, rightPaddle, ball;
			st.dtors.emplace_back(engine->addobject(&leftPaddle));
			st.dtors.emplace_back(engine->addobject(&rightPaddle));
			st.dtors.emplace_back(engine->addobject(&ball));
		});

		engine.run("root");
	}
}
