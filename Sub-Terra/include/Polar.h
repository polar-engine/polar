#pragma once

class Polar {
private:
	bool _initDone = false;
	bool _running = false;
	std::vector<EntityBase<Component> *> _objects;
	void Update(DeltaTicks &);
public:
	EntityBase<System> systems;
	Polar() {}
	virtual ~Polar();

	template<typename T> void AddSystem() {
		if(T::IsSupported()) {
			systems.Add<T>(this);
		} else {
			std::string msg = typeid(T).name() + std::string(": unsupported");
			ENGINE_THROW(msg);
		}
	}

	template<typename T, typename ...Ts> void AddSystem(Ts && ...args) {
		if(T::IsSupported()) {
			systems.Add<T>(this, std::forward<Ts>(args)...);
		} else {
			std::string msg = typeid(T).name() + std::string(": unsupported");
			ENGINE_THROW(msg);
		}
	}

	void AddObject(Object *);
	void Init();
	void Run();
	void Destroy();

	void Quit();
};
