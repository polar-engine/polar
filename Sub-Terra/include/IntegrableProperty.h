#pragma once

#include "Property.h"
#include "Integrable.h"

class IntegrableProperty : public Property {
public:
	typedef std::vector<IntegrableBase *> integrables_type;
private:
	integrables_type integrables;
public:
	template<typename _Integrable> inline void AddIntegrable(Integrable<_Integrable> *integrable) {
		integrables.emplace_back(integrable);
	}

	inline const integrables_type * const GetIntegrables() const { return &integrables; }
};
