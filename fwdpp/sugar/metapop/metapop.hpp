#ifndef __FWDPP_SUGAR_METAPOP_HPP__
#define __FWDPP_SUGAR_METAPOP_HPP__

#include <type_traits>
#include <vector>
#include <fwdpp/forward_types.hpp>
#include <fwdpp/sugar/serialization.hpp>

namespace KTfwd {
  namespace sugar {
    /*!
      Abstraction of what is needed to simulate a single population
      using an individual-based sampler from fwdpp
      
      All that is missing is the mutation_type and the container types.
      
      Does not allow copy construction/assignment!
    */
    template<typename mutation_type,
	     typename mlist,
	     typename glist,
	     typename dipvector,
	     typename vglist,
	     typename vdipvector,
	     typename mvector,
	     typename ftvector,
	     typename lookup_table_type>
    class metapop
    {
      static_assert( std::is_same< typename glist::value_type,
		     KTfwd::gamete_base< typename mlist::value_type, mlist > >::value,
		     "glist::value_type must be same as KTfwd::gamete_base< typename mlist::value_type, mlist >" );
    public:
      //Typedefs for various container
      using mutation_t = mutation_type;
      using gamete_t = typename glist::value_type;
      using dipvector_t = dipvector;
      using vdipvector_t = vdipvector;
      using diploid_t = typename dipvector_t::value_type;
      using mlist_t = mlist;
      using glist_t = glist;
      using vglist_t = vglist;
      using lookup_table_t = lookup_table_type;

      //public, non-const data
      std::vector<unsigned> Ns;
      mlist_t mutations;
      vglist_t metapop_gametes;
      vdipvector_t diploids;
      lookup_table_type mut_lookup;
      mvector fixations;
      ftvector fixation_times;
      
      //! Construct with a list of population sizes
      metapop( std::initializer_list<unsigned> & __Ns ) : Ns(__Ns),
							  mutations(mlist_t()),
							  metapop_gametes(vglist_t()),
							  diploids(vdipvector_t()),
							  mut_lookup(lookup_table_type()),
							  fixations(mvector()),
							  fixation_times(ftvector())
      {
	for( unsigned i = 0 ; i < Ns.size() ; ++i )
	  {
	    metapop_gametes.emplace_back( glist_t(1,gamete_t(2*Ns[i])) );
	    diploids.emplace_back(dipvector_t(Ns[i],std::make_pair( metapop_gametes[i].begin(),metapop_gametes[i].begin())));
	  }
      }

      //! Move constructor
      metatpop( metapop && __m ) : Ns(std::move(__m.Ns)),
				   mutations(std::move(__m.mutations)),
				   metapop_gametes(std::move(__m.metapop_gametes)),
				   diploids(std::move(__m.diploids)),
				   mut_lookup(lookup_table_type()),
				   fixations(std::move(__m.fixations)),
				   fixation_times(std::move(__m.fixation_times))
      {
	//Fill the mutation lookup!
	std::for_each( mutations.begin(), mutations.end(),
		       [this]( const mutation_t & __m ) { mut_lookup.insert(__m.pos); } );
      }

      metapop( metapop & ) = delete;
      metapop( const metapop & ) = delete;
      metapop & operator=(metapop &) = delete;
      metapop & operator=(const metapop &) = delete;
  }
}

#endif
