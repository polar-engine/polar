#pragma once

#include <polar/property/base.h>
#include <polar/support/integrator/integrable.h>
#include <vector>

namespace polar {
	namespace property {
		class integrable : public base {
			using su_integrable_base =
			    polar::support::integrator::integrable_base;
			template<typename... Ts>
			using su_integrable = polar::support::integrator::integrable<Ts...>;

		  public:
			typedef std::vector<su_integrable_base *> integrable_vector_t;

		  private:
			integrable_vector_t integrables;

		  public:
			template<typename _Integrable, typename _Deriv>
			inline void add(su_integrable<_Integrable, _Deriv> *i) {
				integrables.emplace_back(i);
			}

			inline const integrable_vector_t *get() const {
				return &integrables;
			}
		};
	} // namespace property
} // namespace polar
