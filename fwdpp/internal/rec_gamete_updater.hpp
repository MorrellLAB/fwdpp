#ifndef __FWDPP_INTERNAL_REC_GAMETE_UPDATER_HPP__
#define __FWDPP_INTERNAL_REC_GAMETE_UPDATER_HPP__

#include <algorithm>
#include <iterator>
#include <functional>

namespace KTfwd
{
  namespace fwdpp_internal
  {
    template<typename itr_type>
    inline itr_type rec_update_itr( itr_type __first,
				    itr_type __last,
				    const double & val)
    {
      if(__first==__last) return __first;
      return std::upper_bound(__first,__last,
				std::cref(val),
			      [](const double __val,const typename itr_type::value_type __mut) {
				return __val < __mut->pos;
			      });
    }
    
    template< typename itr_type,
	      typename cont_type >
    itr_type rec_gam_updater( itr_type __first, itr_type __last,
			      cont_type & muts,
			      const double & val )
    {
      //O(log_2) comparisons of double plus at most __last - __first copies
      itr_type __ub = rec_update_itr(__first,__last,val);
      /*
	NOTE: the use of insert here
	instead of std::copy(__first,__ub,std::back_inserter(muts));
	Reduced peak RAM use on GCC 4.9.2/Ubuntu Linux.
      */
      muts.insert(muts.end(),__first,__ub);
      return __ub;
    }
  }
}

#endif
