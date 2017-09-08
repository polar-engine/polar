#include <pong/app.h>

namespace pong {
	app::app(polar::core::polar &engine) {
		using namespace polar;
		engine.addstate("root", [] (core::polar *engine, core::state &st) {
			IDType leftPaddle, rightPaddle, ball;
			st.dtors.emplace_back(engine->addobject(&leftPaddle));
			st.dtors.emplace_back(engine->addobject(&rightPaddle));
			st.dtors.emplace_back(engine->addobject(&ball));
		});

		engine.run("root");
	}
}
