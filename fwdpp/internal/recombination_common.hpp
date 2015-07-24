#ifndef __FDWPP_INTERNAL_RECOMBINATION_COMMON_HPP__
#define __FDWPP_INTERNAL_RECOMBINATION_COMMON_HPP__

#include <fwdpp/internal/rec_gamete_updater.hpp>
#include <cassert>
namespace KTfwd {

  namespace fwdpp_internal {

    template<typename double_vec_type,
	     typename gamete_cont_iterator >
    void recombine_gametes( const double_vec_type & pos,
			    gamete_cont_iterator & ibeg,
			    gamete_cont_iterator & jbeg,
			    typename gamete_cont_iterator::value_type::mutation_container & neutral,
			    typename gamete_cont_iterator::value_type::mutation_container & selected)
    {
      assert( std::is_sorted(pos.cbegin(),pos.cend()) );
      short SWITCH = 0;

      using itr_t = typename gamete_cont_iterator::value_type::mutation_container::iterator;
      auto itr = ibeg->mutations.cbegin(),
	jtr = jbeg->mutations.cbegin(),
	itr_s = ibeg->smutations.cbegin(),
	jtr_s = jbeg->smutations.cbegin(),
	itr_e = ibeg->mutations.cend(),
	itr_s_e = ibeg->smutations.cend(),
	jtr_e = jbeg->mutations.cend(),
	jtr_s_e = jbeg->smutations.cend();

      for(const double dummy : pos )
	{
	  if(!SWITCH)
	    {
	      itr = fwdpp_internal::rec_gam_updater(itr,itr_e,
						    neutral,dummy);
	      itr_s = fwdpp_internal::rec_gam_updater(itr_s,itr_s_e,
						      selected,dummy);
	      jtr = std::lower_bound(jtr,jtr_e,
				     dummy,
	       			     [](const typename itr_t::value_type & __mut,const double & __val) {
	       			       return __mut->pos < __val;
	       			     });
	      jtr_s = std::lower_bound(jtr_s,jtr_s_e,
	      			       dummy,
	       			       [](const typename itr_t::value_type & __mut,const double & __val) {
	       				 return (__mut)->pos < __val;
	       			       });
	    }
	  else
	    {
	      jtr = fwdpp_internal::rec_gam_updater(jtr,jtr_e,
						    neutral,dummy);
	      jtr_s = fwdpp_internal::rec_gam_updater(jtr_s,jtr_s_e,
						      selected,dummy);
	      itr = std::lower_bound(itr,itr_e,
				     dummy,
	       			     [](const typename itr_t::value_type & __mut,const double & __val) {
	       			       return __mut->pos < __val;
	       			     });
	      itr_s = std::lower_bound(itr_s,itr_s_e,
	      			       dummy,
	       			       [](const typename itr_t::value_type & __mut,const double & __val) {
	       				 return (__mut)->pos < __val;
	       			       });
	    }
	  SWITCH=!SWITCH;
	}
#ifndef NDEBUG
      using mlist_itr = typename gamete_cont_iterator::value_type::mutation_container::iterator::value_type;
      auto am_I_sorted = [](mlist_itr lhs,mlist_itr rhs){return lhs->pos < rhs->pos;};
      assert(std::is_sorted(neutral.begin(),neutral.end(),std::cref(am_I_sorted)));
      assert(std::is_sorted(selected.begin(),selected.end(),std::cref(am_I_sorted)));
#endif
    }
    
  }
}
#endif
