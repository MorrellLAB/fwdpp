#ifndef __FWDPP_SAMPLE_DIPLOID_HPP__
#define __FWDPP_SAMPLE_DIPLOID_HPP__

#include <utility>
#include <vector>

namespace KTfwd
{
  /*! \brief Sample the next generation of dipliods in an individual-based simulation.  Constant population size case.
     \param r GSL random number generator
     \param gametes Pointer to list of gametes currently in population
     \param diploids Pointer to the vector of parents from which we sample offspring
     \param mutations Pointer to list of mutations currently in population
     \param N_curr The population size
     \param mu The total mutation rate per gamete
     \param mmodel Mutation model policy
     \param rec_pol Recombination model policy
     \param mpolicy Policy determining how new mutations are added to the population
     \param gpolicy_mut Policy determining how new gametes are added to population after a mutation event
     \param ff Policy calculating the fitness of a diploid
     \param mp Policy determining how to remove mutations from a diploid (e.g., removing fixed and/or lost mutations)
     \param f Probability that a mating is a selfing event
     \note diploids will be updated to reflect the new diploid genotypes post-sampling (the descedants).  Gametes will be changed by mutation, recombination, and sampling.  Mutations will be changed by mutation and sampling.
     \return The mean fitness of the parental generation
     \example diploid_ind.cc 
     \example pfix.cc
     \example diploid_fixed_sh_ind_lambda.cc
   */
  template< typename gamete_type,
	    typename gamete_list_type_allocator,
	    typename mutation_list_type_allocator,
	    typename diploid_geno_t,
	    typename diploid_vector_type_allocator,
	    typename diploid_fitness_function,
	    typename mutation_removal_policy,
	    typename mutation_model,
	    typename recombination_policy,
	    typename mutation_insertion_policy,
	    typename gamete_insertion_policy,
	    template<typename,typename> class gamete_list_type,
	    template<typename,typename> class mutation_list_type,
	    template<typename,typename> class diploid_vector_type>
  double
  sample_diploid(gsl_rng * r,
		 gamete_list_type<gamete_type,gamete_list_type_allocator > * gametes,
		 diploid_vector_type<diploid_geno_t,diploid_vector_type_allocator> * diploids,
		 mutation_list_type<typename gamete_type::mutation_type,mutation_list_type_allocator > * mutations, 
		 const unsigned & N_curr, 
		 const double & mu,
		 const mutation_model & mmodel,
		 const recombination_policy & rec_pol,
		 const mutation_insertion_policy & mpolicy,
		 const gamete_insertion_policy & gpolicy_mut,
		 const diploid_fitness_function & ff,
		 const mutation_removal_policy & mp,
		 const double & f = 0);
 
  /*! \brief Sample the next generation of dipliods in an individual-based simulation.  Changing population size case.
    \param r GSL random number generator
    \param gametes Pointer to list of gametes currently in population
    \param diploids Pointer to the vector of parents from which we sample offspring
    \param mutations Pointer to list of mutations currently in population
    \param N_curr The population size in the current generation
    \param N_next The population size in the next generation
    \param mu The total mutation rate per gamete
    \param mmodel Mutation model policy
    \param rec_pol Recombination model policy
    \param mpolicy Policy determining how new mutations are added to the population
    \param gpolicy_mut Policy determining how new gametes are added to population after a mutation event
    \param ff Policy calculating the fitness of a diploid
    \param mp Policy determining how to remove mutations from a diploid (e.g., removing fixed and/or lost mutations)
    \param f Probability that a mating is a selfing event
    
     \note diploids will be updated to reflect the new diploid genotypes post-sampling (the descedants).  Gametes will be changed by mutation, recombination, and sampling.  Mutations will be changed by mutation and sampling.
     \return The mean fitness of the parental generation
     \example bneck_selection_ind.cc
   */
template< typename gamete_type,
	  typename gamete_list_type_allocator,
	  typename mutation_list_type_allocator,
	  typename diploid_geno_t,
	  typename diploid_vector_type_allocator,
	  typename diploid_fitness_function,
	  typename mutation_removal_policy,
	  typename mutation_model,
	  typename recombination_policy,
	  typename mutation_insertion_policy,
	  typename gamete_insertion_policy,
	  template<typename,typename> class gamete_list_type,
	  template<typename,typename> class mutation_list_type,
	  template<typename,typename> class diploid_vector_type>
  double
  sample_diploid(gsl_rng * r,
		 gamete_list_type<gamete_type,gamete_list_type_allocator > * gametes,
		 diploid_vector_type<diploid_geno_t, diploid_vector_type_allocator> * diploids,
		 mutation_list_type<typename gamete_type::mutation_type,mutation_list_type_allocator > * mutations, 
		 const unsigned & N_curr, 
		 const unsigned & N_next, 
		 const double & mu,
		 const mutation_model & mmodel,
		 const recombination_policy & rec_pol,
		 const mutation_insertion_policy & mpolicy,
		 const gamete_insertion_policy & gpolicy_mut,
		 const diploid_fitness_function & ff,
		 const mutation_removal_policy & mp,
		 const double & f = 0);

  /*! \brief Evolve a metapopulation where demes are not changing size.  For individual-based sims.
    Evolve a metapopulation where demes are not changing size.  For individual-based sims.
     \param r GSL random number generator
     \param metapop Pointer to vector of lists of gametes.
     \param dipliods Pointer to the vector of vectors of parents from which we sample offspring
     \param mutations Pointer to list of mutations currently in population
     \param N_curr The population sizes.  There must be diploids->size() values in this array
     \param mu The total mutation rate per gamete
     \param mmodel Mutation model policy
     \param rec_pol Recombination model policy
     \param mpolicy Policy determining how new mutations are added to the population
     \param gpolicy_mut Policy determining how new gametes are added to population after a mutation event
     \param ffs Container of fitness functions.  One for each deme.
     \param mp Policy determining how to remove mutations from a diploid (e.g., removing fixed and/or lost mutations)
     \param mig Migration policy.  This function/function object must take a single size_t (values 0 to metapop->size()-1).  If no migration event occurs, the passed value is returned.  Otherwise, a size_t representing the index of the deme from which the other parent comes (aka the migrant) is returned.
     \param f Probability that a mating is a selfing event.  This is an array, with 1 f per deme.

     \note diploids will be updated to reflect the new diploid genotypes post-sampling (the descedants).  Gametes will be changed by mutation, recombination, and sampling.  Mutations will be changed by mutation and sampling.
     \return The mean fitness of the parental generation

     \example migsel_ind.cc
   */
template< typename gamete_type,
	  typename metapop_diploid_vector_type_allocator,
	  typename gamete_list_type_allocator,
	  typename mutation_list_type_allocator,
	  typename diploid_geno_t,
	  typename diploid_vector_type_allocator,
	  typename diploid_fitness_function_container,
	  typename mutation_removal_policy,
	  typename mutation_model,
	  typename recombination_policy,
	  typename migration_policy,
	  typename mutation_insertion_policy,
	  typename gamete_insertion_policy,
	  template<typename,typename> class gamete_list_type,
	  template<typename,typename> class mutation_list_type,
	  template<typename,typename> class diploid_vector_type,
	  template<typename,typename> class metapop_diploid_vector_type>
std::vector< double >
sample_diploid(gsl_rng * r,
	       gamete_list_type<gamete_type,gamete_list_type_allocator> * metapop,
	       metapop_diploid_vector_type < diploid_vector_type<diploid_geno_t,diploid_vector_type_allocator>,metapop_diploid_vector_type_allocator > * diploids,
	       mutation_list_type<typename gamete_type::mutation_type,mutation_list_type_allocator > * mutations, 
	       const unsigned * N_curr, 
	       const double & mu,
	       const mutation_model & mmodel,
	       const recombination_policy & rec_pol,
	       const mutation_insertion_policy & mpolicy,
	       const gamete_insertion_policy & gpolicy_mut,
	       const diploid_fitness_function_container & ffs,
	       const mutation_removal_policy & mp,
	       const migration_policy & mig,
	       const double * f = nullptr);

  /*! \brief Evolve a metapopulation where demes may be changing size.  For individual-based sims.
    Evolve a metapopulation where demes may be changing size.  For individual-based sims.
    \param r GSL random number generator
    \param metapop Pointer to vector of lists of gametes.
    \param diploids Pointer to the vector of vectors of parents from which we sample offspring
    \param mutations Pointer to list of mutations currently in population
    \param N_curr The current population sizes.  There must be diploids->size() values in this array
    \param N_next The population sizes in the daughter generation.  There must be diploids->size() values in this array
    \param mu The total mutation rate per gamete
    \param mmodel Mutation model policy
    \param rec_pol Recombination model policy
    \param mpolicy Policy determining how new mutations are added to the population
    \param gpolicy_mut Policy determining how new gametes are added to population after a mutation event
    \param ffs Container of fitness functions.  One for each deme.
    \param mp Policy determining how to remove mutations from a diploid (e.g., removing fixed and/or lost mutations)
    \param mig Migration policy.  This function/function object must take a single size_t (values 0 to metapop->size()-1).  If no migration event occurs, the passed value is returned.  Otherwise, a size_t representing the index of the deme from which the other parent comes (aka the migrant) is returned.
    \param f Probability that a mating is a selfing event.  This is an array, with 1 f per deme.
    
    \note diploids will be updated to reflect the new diploid genotypes post-sampling (the descedants).  Gametes will be changed by mutation, recombination, and sampling.  Mutations will be changed by mutation and sampling.
    \return The mean fitness of the parental generation
  */
template< typename gamete_type,
	  typename metapop_diploid_vector_type_allocator,
	  typename gamete_list_type_allocator,
	  typename mutation_list_type_allocator,
	  typename diploid_geno_t,
	  typename diploid_vector_type_allocator,
	  typename diploid_fitness_function_container,
	  typename mutation_removal_policy,
	  typename mutation_model,
	  typename recombination_policy,
	  typename migration_policy,
	  typename mutation_insertion_policy,
	  typename gamete_insertion_policy,
	  template<typename,typename> class gamete_list_type,
	  template<typename,typename> class mutation_list_type,
	  template<typename,typename> class diploid_vector_type,
	  template<typename,typename> class metapop_diploid_vector_type>
std::vector< double >
sample_diploid(gsl_rng * r,
	       gamete_list_type<gamete_type,gamete_list_type_allocator> * metapop,
	       metapop_diploid_vector_type < diploid_vector_type<diploid_geno_t, diploid_vector_type_allocator>, metapop_diploid_vector_type_allocator > * diploids,
	       mutation_list_type<typename gamete_type::mutation_type,mutation_list_type_allocator > * mutations, 
	       const unsigned * N_curr, 
	       const unsigned * N_next, 
	       const double & mu,
	       const mutation_model & mmodel,
	       const recombination_policy & rec_pol,
	       const mutation_insertion_policy & mpolicy,
	       const gamete_insertion_policy & gpolicy_mut,
	       const diploid_fitness_function_container & ffs,
	       const mutation_removal_policy & mp,
	       const migration_policy & mig,
	       const double * f = nullptr);

  /*! \brief Single deme, multilocus model, changing population size
   */
  template< typename diploid_geno_t,
	    typename gamete_type,
	    typename gamete_list_type_allocator,
	    typename mutation_list_type_allocator,
	    typename diploid_vector_type_allocator,
	    typename locus_vector_type_allocator,
	    typename diploid_fitness_function,
	    typename mutation_removal_policy,
	    typename mutation_model_container,
	    typename recombination_policy_container,
	    typename mutation_insertion_policy,
	    typename gamete_insertion_policy,
	    typename bw_locus_rec_fxn,
	    template<typename,typename> class gamete_list_type,
	    template<typename,typename> class mutation_list_type,
	    template<typename,typename> class diploid_vector_type,
	    template<typename,typename> class locus_vector_type>
  double
  sample_diploid(gsl_rng * r,
		 gamete_list_type<gamete_type, gamete_list_type_allocator> * gametes,
		 diploid_vector_type<locus_vector_type< diploid_geno_t, locus_vector_type_allocator>, diploid_vector_type_allocator> * diploids,
		 mutation_list_type<typename gamete_type::mutation_type,mutation_list_type_allocator > * mutations, 
		 const unsigned & N_curr, 
		 const unsigned & N_next, 
		 const double * mu,
		 const mutation_model_container & mmodel,
		 const recombination_policy_container & rec_policies,
		 const double * r_between_loci,
		 const bw_locus_rec_fxn & blrf,
		 const mutation_insertion_policy & mpolicy,
		 const gamete_insertion_policy & gpolicy_mut,
		 const diploid_fitness_function & ff,
		 const mutation_removal_policy & mp,
		 const double & f = 0);
  
  /*! \brief Single deme, multilocus model, constant population size
    \example diploid_ind_2locus.cc
  */
  //single deme, constant N
  template< typename diploid_geno_t,
	    typename gamete_type,
	    typename gamete_list_type_allocator,
	    typename mutation_list_type_allocator,
	    typename diploid_vector_type_allocator,
	    typename locus_vector_type_allocator,
	    typename diploid_fitness_function,
	    typename mutation_removal_policy,
	    typename mutation_model_container,
	    typename recombination_policy_container,
	    typename mutation_insertion_policy,
	    typename gamete_insertion_policy,
	    typename bw_locus_rec_fxn,
	    template<typename,typename> class gamete_list_type,
	    template<typename,typename> class mutation_list_type,
	    template<typename,typename> class diploid_vector_type,
	    template<typename,typename> class locus_vector_type>
  double
  sample_diploid(gsl_rng * r,
		 gamete_list_type<gamete_type, gamete_list_type_allocator> * gametes,
		 diploid_vector_type<locus_vector_type< diploid_geno_t, locus_vector_type_allocator>, diploid_vector_type_allocator> * diploids,
		 mutation_list_type<typename gamete_type::mutation_type,mutation_list_type_allocator > * mutations, 
		 const unsigned & N,
		 const double * mu,
		 const mutation_model_container & mmodel,
		 const recombination_policy_container & rec_policies,
		 const double * r_between_loci,
		 const bw_locus_rec_fxn & blrf,
		 const mutation_insertion_policy & mpolicy,
		 const gamete_insertion_policy & gpolicy_mut,
		 const diploid_fitness_function & ff,
		 const mutation_removal_policy & mp,
		 const double & f = 0);
}

#include <fwdpp/sample_diploid.tcc>
#endif

