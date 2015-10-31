#pragma once

class HumanPlayerController : public System {
private:
	IDType object;
	Point2 orientVel;
	Point2 orientRot;
	long double accum = 0.0;
	float velocity = 10.0f;
protected:
	virtual void Init() override;
	virtual void Update(DeltaTicks &) override;
public:
	static bool IsSupported() { return true; }
	HumanPlayerController(Polar *engine, const IDType object) : System(engine), object(object) {}
};