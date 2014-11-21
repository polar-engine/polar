#pragma once

class Polar {
private:
	bool _initDone = false;
	std::vector<EntityBase<Component> *> _objects;
	void Init();
	void Update(DeltaTicks &);
	void Destroy();
public:
	EntityBase<System> systems;
	Polar() {}
	virtual ~Polar();

	template<typename T> void AddSystem() {
		systems.Add<T>(this);
	}

	template<typename T, typename ...Ts> void AddSystem(Ts && ...args) {
		systems.Add<T>(this, std::forward<Ts>(args)...);
	}

	void AddObject(Object *);
	void Run();
};
