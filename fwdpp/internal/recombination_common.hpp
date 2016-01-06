#ifndef __FDWPP_INTERNAL_RECOMBINATION_COMMON_HPP__
#define __FDWPP_INTERNAL_RECOMBINATION_COMMON_HPP__

#include <fwdpp/internal/rec_gamete_updater.hpp>
#include <cassert>
namespace KTfwd {

  namespace fwdpp_internal {

    template<typename double_vec_type,
	     typename gcont_t,
	     typename mcont_t>
    void recombine_gametes( const double_vec_type & pos,
			    const std::size_t ibeg,
			    const std::size_t jbeg,
			    gcont_t & gametes,
			    const mcont_t & mutations,
			    std::vector<std::size_t> & neutral,
			    std::vector<std::size_t> & selected)
    {
      assert( std::is_sorted(pos.cbegin(),pos.cend()) );
      short SWITCH = 0;

      auto itr = gametes[ibeg].mutations.cbegin(),
	jtr = gametes[jbeg].mutations.cbegin(),
	itr_s = gametes[ibeg].smutations.cbegin(),
	jtr_s = gametes[jbeg].smutations.cbegin(),
	itr_e = gametes[ibeg].mutations.cend(),
	itr_s_e = gametes[ibeg].smutations.cend(),
	jtr_e = gametes[jbeg].mutations.cend(),
	jtr_s_e = gametes[jbeg].smutations.cend();
      
      for(const double dummy : pos )
	{
	  if(!SWITCH)
	    {
	      itr = fwdpp_internal::rec_gam_updater(itr,itr_e,mutations,
						    neutral,dummy);
	      itr_s = fwdpp_internal::rec_gam_updater(itr_s,itr_s_e,mutations,
						      selected,dummy);
	      jtr = fwdpp_internal::rec_update_itr(jtr,jtr_e,mutations,dummy);
	      jtr_s = fwdpp_internal::rec_update_itr(jtr_s,jtr_s_e,mutations,dummy);
	    }
	  else
	    {
	      jtr = fwdpp_internal::rec_gam_updater(jtr,jtr_e,mutations,
						    neutral,dummy);
	      jtr_s = fwdpp_internal::rec_gam_updater(jtr_s,jtr_s_e,mutations,
						      selected,dummy);
	      itr = fwdpp_internal::rec_update_itr(itr,itr_e,mutations,dummy);
	      itr_s = fwdpp_internal::rec_update_itr(itr_s,itr_s_e,mutations,dummy);
	    }
	  SWITCH=!SWITCH;
	}
#ifndef NDEBUG
      if( ! std::is_sorted( gametes[ibeg].mutations.begin(),gametes[ibeg].mutations.end(),[&mutations](size_t i,size_t j) {
	    return mutations[i].pos < mutations[j].pos;
	  }) )
	{
	  for( const auto & m : gametes[ibeg].mutations )
	    {
	      std::cerr << mutations[m].pos << '\n';
	    }
	}
#endif
      
      assert( std::is_sorted( gametes[ibeg].mutations.begin(),gametes[ibeg].mutations.end(),[&mutations](size_t i,size_t j) {
	    return mutations[i].pos < mutations[j].pos;
	  }) );
      assert( std::is_sorted( gametes[jbeg].smutations.begin(),gametes[jbeg].smutations.end(),[&mutations](size_t i,size_t j) {
	    return mutations[i].pos < mutations[j].pos;
	  }) );
    }    
  }
}
#endif
