/*
  \include diploid_fixed_sh_ind_lambda.cc
  
  Same as diploid_fixed_sh.cc, but individual-based
*/

#include <fwdpp/diploid.hh>
#include <Sequence/SimData.hpp>
#include <numeric>
#include <functional>
#include <cassert>
#include <iomanip>
#include <fwdpp/sugar/infsites.hpp>
#define SINGLEPOP_SIM
//the type of mutation
using mtype = KTfwd::mutation;
#include <common_ind.hpp>

using namespace KTfwd;

int main(int argc, char ** argv)
{
  if (argc != 11)
    {
      std::cerr << "Too few arguments.\n"
		<< "Usage: diploid_fixed_sh_ind_lambda N theta_neutral theta_deleterious rho s h ngens samplesize nreps seed\n";
      exit(10);
    }
  int argument=1;
  const unsigned N = atoi(argv[argument++]);
  const double theta_neutral = atof(argv[argument++]);
  const double theta_del = atof(argv[argument++]);
  const double rho = atof(argv[argument++]);
  const double s = atof(argv[argument++]);
  const double h = atof(argv[argument++]);
  const unsigned ngens = atoi(argv[argument++]);
  const unsigned samplesize1 = atoi(argv[argument++]);
  int nreps = atoi(argv[argument++]);
  const unsigned seed = atoi(argv[argument++]);

  const double mu_neutral = theta_neutral/double(4*N);
  const double mu_del = theta_del/double(4*N);
  const double littler = rho/double(4*N);

  std::copy(argv,argv+argc,std::ostream_iterator<char*>(std::cerr," "));
  std::cerr << '\n';

  GSLrng r(seed);

  unsigned twoN = 2*N;

  KTfwd::site_dependent_fitness fitness_model;
  while(nreps--)
    {
      singlepop_t pop(N);
      unsigned generation;
      double wbar=1;
      for( generation = 0; generation < ngens; ++generation )
	{
#ifndef NDEBUG
	  for( auto itr = pop.gametes.begin(); 
	       itr != pop.gametes.end() ; ++itr )
	    {
	      assert( itr->n > 0 );
	    }
#endif
	  assert(KTfwd::check_sum(pop.gametes,2*N));
	  wbar = KTfwd::sample_diploid(r.get(),
				       &pop.gametes,
				       &pop.diploids,
				       &pop.mutations,
				       N,
				       mu_neutral+mu_del,
      				       /*
      					 The mutation model (KTfwd::infsites) will be applied by
					 sample_diploid in order to add mutations to gametes each generation.
      				       */
				       std::bind(KTfwd::infsites(),r.get(),&pop.mut_lookup,
						 mu_neutral,mu_del,[&r](){return gsl_rng_uniform(r.get());},[&s](){return s;},[&h](){return h;}),
				       //The recombination policy must take two non-const iterators from the glist
				       [&](singlepop_t::glist_t::iterator & g1,
					   singlepop_t::glist_t::iterator & g2,
					   /*
					     This is an oddity of lambdas: the user will either have to dig into fwdpp's internals
					     to determine what this type is, or rely on decltype, which in this case is asking about the
					     return type of the function creating the lookup table.
					   */
					   decltype(KTfwd::fwdpp_internal::gamete_lookup_table(&pop.gametes)) & gamete_lookup) {
					 return KTfwd::recombine_gametes(r.get(),littler,&pop.gametes,g1,g2,gamete_lookup,
									 std::ref(pop.neutral),std::ref(pop.selected),
									 //This nested lambda is our genetic map: uniform on interval (0,1]
									 [&](){return gsl_rng_uniform(r.get());}); },
				       //The mutation insertion policy takes a const singlepop_t::mutation_t and a non-const pointer to an singlepop_t::mlist_t
				       [](const singlepop_t::mutation_t & m,singlepop_t::mlist_t * __mutations) { return __mutations->insert(__mutations->end(),m); },
				       //The gamete insertion policy takes a const singlepop_t::gamete_t and a non-const pointer to a glist
				       []( const singlepop_t::gamete_t & g,singlepop_t::glist_t * __gametes) { return __gametes->insert(__gametes->end(),g); },
				       //Our fitness model takes two const singlepop_t::glist_t::const_iterators  and two policies as arguments
				       [&](const singlepop_t::glist_t::const_iterator & g1, const singlepop_t::glist_t::const_iterator &g2) {
					 return std::cref(fitness_model)(g1,g2,
									 //The first policy is what to do for a homozygous mutation
									 //The "homozygote" policy must take a non-const reference to a double
									 //and a const singlepop_t::mlist_t::iterator as arguments
									 [](double & fitness,const singlepop_t::mlist_t::iterator & mut) { 
									   fitness *= (1. + 2.*mut->s);
									 },
									 //The second is what to do for a heterozygous mutation
									 //The "heterozygote" policy must take a non-const reference to a double
									 //and a const singlepop_t::mlist_t::iterator as arguments
									 [](double & fitness,const singlepop_t::mlist_t::iterator & mut) {
									   fitness *= (1. + mut->h*mut->s);
									 });},
				       //Finally, mutations will be removed from gametes if they are extinct or fixed
				       [&N](const singlepop_t::mlist_t::iterator & i){ return (!i->n || i->n == 2*N); });
	  KTfwd::remove_fixed_lost(&pop.mutations,&pop.fixations,&pop.fixation_times,&pop.mut_lookup,generation,2*N);
	  assert(KTfwd::check_sum(pop.gametes,2*N));
	}
      Sequence::SimData neutral_muts,selected_muts;

      //Take a sample of size samplesize1.  Two data blocks are returned, one for neutral mutations, and one for selected
      std::pair< std::vector< std::pair<double,std::string> >,
		 std::vector< std::pair<double,std::string> > > sample = ms_sample_separate(r.get(),&pop.diploids,samplesize1);

      neutral_muts.assign( sample.first.begin(), sample.first.end() );
      selected_muts.assign( sample.second.begin(), sample.second.end() );

      std::cout << neutral_muts << '\n' << selected_muts << '\n';
    }
}
