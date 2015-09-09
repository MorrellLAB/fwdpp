#ifndef __FWDPP_RECOMBINATION_HPP__
#define __FWDPP_RECOMBINATION_HPP__

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

namespace KTfwd
{
  /*! \brief recombination for individual-based forward simulations
    \param r GSL random number generator
    \param littler The probability of a single recombination event between g1 and g2
    \param gametes Pointer to the list of gametes in the population
    \param g1 Iterator to the first gamete involved in the recombination event
    \param g2 Iterator to the second gamete involved in the recombination event
    \param mf Recombination policy which generates crossover positions
    \param gamete_lookup The return value of KTfwd::fwdpp_internal::gamete_lookup_table, which gets passed in via sample_diploid
    \note g1 will be changed
    \note The type of g1 and g2 is gamete_list_type<gamete_type,list_type_allocator >::iterator
    \note The return value may be 0 even if littler is large.  The code recognizes when crossovers could not modify the gametes, and the function returns when such cases are found
    \return The number of crossovers that happened between g1 and g2 (which is Poisson with mean littler)
   */
  template< typename iterator_type,
	    typename recombination_map,
	    typename list_type_allocator,
	    typename glookup_t,
	    template<typename,typename> class list_type>
  unsigned recombine_gametes( gsl_rng * r,
			      const double & littler,
			      list_type< typename iterator_type::value_type,list_type_allocator > * gametes,
			      iterator_type & g1,
			      iterator_type & g2,
			      glookup_t & gamete_lookup,
			      typename iterator_type::value_type::mutation_container & neutral,
			      typename iterator_type::value_type::mutation_container & selected,
			      const recombination_map & mf);

  /*!
    Overload for fixed xover positions.
    Typically, this is called by the version taking 
    a recombination policy as an argument.

    If you wish to call this version directly,
    only do so if length pos > 1, pos is sorted
    in ascending order, and the last value in 
    pos is std::numeric_limits<double>::max(),
    which is assumed to be a terminating value 
    larger than any possible value for a mutation's position.

    \param pos A vector (with interface of std::vector) containing recombination breakpoints.  See note below.
    \param gametes A container of the gametes segregating in the population
    \param g1 An iterator, derived from gametes, representing one parental gamete.
    \param g2 An iterator, derived from gametes, representing the other parental gamete.
    \param gamete_lookup The return value of KTfwd::fwdpp_internal::gamete_lookup_table, which gets passed in via sample_diploid
    \return The number of breakpoints, which equals pos.size() - 1, as that is fixed in this case.
    \note The vector pos must be sorted (ascending order) and must contain the value std::numeric_limits<double>::max() as a terminating value.
  */
  template< typename iterator_type,
	    typename list_type_allocator,
	    typename vector_type_allocator,
	    typename glookup_t,
	    template<typename,typename> class vector_type,
	    template<typename,typename> class list_type>
  unsigned recombine_gametes( const vector_type< double, vector_type_allocator > & pos,
			      list_type< typename iterator_type::value_type,list_type_allocator > * gametes,
			      iterator_type & g1,
			      iterator_type & g2,
			      glookup_t & gamete_lookup,
			      typename iterator_type::value_type::mutation_container & neutral,
			      typename iterator_type::value_type::mutation_container & selected );

}
#endif // __FWDPP_RECOMBINATION_HPP__ 
#include <fwdpp/recombination.tcc>


