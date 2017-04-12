#pragma once

class HumanPlayerController : public System {
private:
	boost::shared_ptr<Destructor> timeDtor;
	boost::shared_ptr<Destructor> soundDtor;
	IDType timeID = 0;

	IDType object;
	Point2 orientVel;
	Point2 orientRot;
	long double accum = 0.0;
	float velocity = 10.0f;
	bool moveForward = false, moveBackward = false, moveLeft = false, moveRight = false;
protected:
	virtual void Init() override;
	virtual void Update(DeltaTicks &) override;
public:
	float time = 0.0f;

	static bool IsSupported() { return true; }
	HumanPlayerController(Polar *engine, const IDType object) : System(engine), object(object) {}
};