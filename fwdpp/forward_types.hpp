/*!
  \file forward_types.hpp
*/
#ifndef _FORWARD_TYPES_HPP_
#define _FORWARD_TYPES_HPP_

#include <limits>
#include <vector>
#include <list>
#include <cmath>
#include <type_traits>

namespace KTfwd
{
  /*! \brief Base class for mutations
    At minimum, a mutation must contain a position and a count in the population.	
    You can derive from this class, for instance to add selection coefficients,
    counts in different sexes, etc.
  */
  struct mutation_base
  {
    /// Mutation position
    mutable double pos;
    /// Count of mutation in the population
    unsigned n;
    /// Is the mutation neutral or not?
    bool neutral;
    /// Used internally (don't worry about it for now...)
    bool checked;
    mutation_base(const double & position, const unsigned & count, const bool & isneutral = true)
      : pos(position),n(count),neutral(isneutral),checked(false)
    {	
    }
    virtual ~mutation_base(){}
    mutation_base( mutation_base & ) = default;
    mutation_base( mutation_base const & ) = default;
    mutation_base( mutation_base && ) = default;
    mutation_base & operator=(mutation_base &) = default;
    mutation_base & operator=(mutation_base const &) = default;
    mutation_base & operator=(mutation_base &&) = default;
  };

  struct mutation : public mutation_base
  //!The simplest mutation type, adding just a selection coefficient and dominance to the interface
  {
    /// selection coefficient
    mutable double s;
    /// dominance coefficient
    mutable double h;
    mutation( const double & position, const double & sel_coeff,const unsigned & count,
	      const double & dominance = 0.5) 
      : mutation_base(position,count,(sel_coeff==0)),s(sel_coeff),h(dominance)
    {
    }
    bool operator==(const mutation & rhs) const
    {
      return( std::fabs(this->pos-rhs.pos) <= std::numeric_limits<double>::epsilon() &&
	      this->s == rhs.s );
    }
  };

  template<typename mut_type,
	   typename list_type = std::list<mut_type> >
  struct gamete_base
  /*! \brief Base class for gametes.
    A gamete is a container of pointers (iterators) to mutations + a count in the population
    \note neutral and non-neutral mutations are stored in separate containers
  */
  {
    static_assert( std::is_base_of<mutation_base,mut_type>::value,
		   "mut_type must be derived from KTfwd::mutation_base" );
    /// Count in population
    unsigned n;
    using mutation_type = mut_type;
    using mutation_list_type = list_type;
    using mutation_list_type_iterator = typename list_type::iterator;
    using mutation_container = std::vector< mutation_list_type_iterator >;
    using mcont_iterator = typename mutation_container::iterator;
    using mcont_const_iterator = typename mutation_container::const_iterator;
    /// mutations is for neutral mutations, smutations for selected...
    mutation_container mutations,smutations;
    gamete_base(const unsigned & icount) : n(icount),mutations( mutation_container() ),smutations(mutation_container())
    {
    }
    gamete_base(const unsigned & icount, const mutation_container & n,
		const mutation_container & s) : n(icount),mutations(n),smutations(s)
    {
    }
    virtual ~gamete_base() noexcept {}
    gamete_base( gamete_base & ) = default;
    gamete_base( gamete_base const & ) = default;
    //! This seems better for large simulations
    gamete_base( gamete_base && ) = default;
    // This may have an edge for smaller sims:
    /*
      gamete_base( gamete_base && rhs) noexcept : n(std::move(rhs.n)),mutations(std::move(rhs.mutations)),smutations(std::move(rhs.mutations))
      {
      mutations.shrink_to_fit();
      smutations.shrink_to_fit();
      }
    */
    gamete_base & operator=(gamete_base &) = default;
    gamete_base & operator=(gamete_base const &) = default;
    gamete_base & operator=(gamete_base &&) = default;
    /*! \brief Equality operation
      \note Given that mutations and smutations contains ITERATORS to actual mutations,
      operator== does not need to be defined for the corresponding mutation type
    */
    inline bool operator==(const gamete_base<mut_type,list_type> & rhs) const
    {
      return(this->mutations == rhs.mutations && this->smutations == rhs.smutations);
    }
  };

  //! The simplest gamete adds nothing to the interface of the base class.
  using gamete = gamete_base<mutation>;

}
#endif /* _FORWARD_TYPES_HPP_ */
