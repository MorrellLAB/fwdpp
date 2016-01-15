/*!
  \file forward_types.hpp
  \defgroup basicTypes Mutation and gamete data types
  These are the basic types common to all simulations: mutations and gametes.  See @ref md_md_policies for more details.
*/
#ifndef _FORWARD_TYPES_HPP_
#define _FORWARD_TYPES_HPP_

#include <limits>
#include <vector>
#include <cmath>
#include <cstdint>
#include <type_traits>
#include <fwdpp/tags/gamete_tags.hpp>
#include <fwdpp/tags/tags.hpp>

namespace KTfwd
{
  //! The unsigned integer type is 32 bits
  using uint_t = std::uint32_t;

  /*! \brief Base class for mutations
    At minimum, a mutation must contain a position and a count in the population.
    You can derive from this class, for instance to add selection coefficients,
    counts in different sexes, etc.
    \ingroup basicTypes
    \note See @ref TutMut in @ref md_md_policies for more detail on how to extend this type
  */
  struct mutation_base
  {
    /// Mutation position
    double pos;
    /// Count of mutation in the population
    //uint_t n;
    /// Is the mutation neutral or not?
    bool neutral;
    /// Used internally (don't worry about it for now...)
    //bool checked;
    mutation_base(const double & position, const bool & isneutral = true) noexcept
      : pos(position),neutral(isneutral)
    {
    }
    virtual ~mutation_base() noexcept {}
    mutation_base( mutation_base & ) = default;
    mutation_base( mutation_base const & ) = default;
    mutation_base( mutation_base && ) = default;
    mutation_base & operator=(mutation_base &) = default;
    mutation_base & operator=(mutation_base const &) = default;
    mutation_base & operator=(mutation_base &&) = default;
  };

  struct mutation : public mutation_base
  /*!
    \brief The simplest mutation type, adding just a selection coefficient and dominance to the interface
    \ingroup basicTypes
  */
  {
    /// selection coefficient
    double s;
    /// dominance coefficient
    double h;
    mutation( const double & position, const double & sel_coeff,
	      const double & dominance = 0.5) noexcept
      : mutation_base(position,(sel_coeff==0)),s(sel_coeff),h(dominance)
    {
    }
    bool operator==(const mutation & rhs) const
    {
      return( std::fabs(this->pos-rhs.pos) <= std::numeric_limits<double>::epsilon() &&
	      this->s == rhs.s );
    }
  };

  /*! \brief Base class for gametes.

    A gamete is a container of pointers (iterators) to mutations + a count in the population.

    The template parameter types are:
    mut_type = the mutation type to be used.  Must be a model of KTfwd::mutation_base
    list_type = the (doubly-linked) list that mutations are stored in.  This is mainly used for defining types for this class
    tag_type = A type that can be used as a "dispatch tag".  Currently, these are not used elsewhere in the library, but they may
    be in the future, or this may disappear in future library releases.  The current default (KTfwd::tags::standard_gamete) maintains
    backwards compatibility with previous library versions and does not affect compilation of existing programs based on the library.

    \note The typical use of this class is simply to define your mutation type (see @ref md_md_policies)
    and then use a typedef to define your gamete type in the simulations:
    \code
    using gamete_t = KTfwd::gamete_base<mutation_type>
    \endcode
    See @ref md_md_policies for examples of this.
    \ingroup basicTypes
  */
  template<typename TAG = void>
  struct gamete_base
  {
    //! Count in population
    uint_t n;
    //! Dispatch tag type
    using gamete_tag = TAG;
    using index_t = std::size_t;
    using mutation_container = std::vector<index_t>;
    //! Container of mutations not affecting trait value/fitness ("neutral mutations")
    mutation_container mutations;
    //! Container of mutations affecting trait value/fitness ("selected" mutations")
    mutation_container smutations;

    /*! @brief Constructor
      \param icount The number of occurrences of this gamete in the population
    */
    gamete_base(const uint_t & icount) noexcept : n(icount),mutations( mutation_container() ),smutations(mutation_container())
    {
    }

    /*! @brief Constructor
      \param icount The number of occurrences of this gamete in the population
      \param n A container of mutations not affecting trait value/fitness
      \param s A container of mutations affecting trait value/fitness
    */
    gamete_base(const uint_t & icount, const mutation_container & n,
		const mutation_container & s) noexcept : n(icount),mutations(n),smutations(s)
    {
    }
    //! Destructor is virtual, so you may inherit from this type
    virtual ~gamete_base() noexcept {}
    //! Copy constructor
    gamete_base( gamete_base & ) = default;
    //! Copy constructor
    gamete_base( gamete_base const & ) = default;
    //! Move constructor
    gamete_base( gamete_base && ) = default;

    //! Assignment operator
    gamete_base & operator=(gamete_base &) = default;
    //! Assignment operator
    gamete_base & operator=(gamete_base const &) = default;
    //! Move assignment operator
    gamete_base & operator=(gamete_base &&) = default;
    /*! \brief Equality operation
    */
    inline bool operator==(const gamete_base<TAG> & rhs) const
    {
      return(this->mutations == rhs.mutations && this->smutations == rhs.smutations);
    }
  };

  /// Default gamete type
  using gamete = gamete_base<void>;
}
#endif /* _FORWARD_TYPES_HPP_ */
