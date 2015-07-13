/*! \file sugar2.cc
  \ingroup unit 
  \brief Testing KTfwd::metapop and KTfwd::metapop_serialized
*/
#define BOOST_TEST_MODULE sugarTest2
#define BOOST_TEST_DYN_LINK 

#include <config.h>
#include <algorithm>
#include <boost/test/unit_test.hpp>
#include <fwdpp/diploid.hh>
#ifndef USE_STANDARD_CONTAINERS //from config.h
#define FWDPP_SUGAR_USE_BOOST
#endif
#include <fwdpp/sugar/GSLrng_t.hpp>
#include <fwdpp/sugar/metapop.hpp>
#include <fwdpp/sugar/infsites.hpp>

using mutation_with_age = KTfwd::popgenmut;
using mwriter = KTfwd::mutation_writer;
using mreader = KTfwd::mutation_reader<mutation_with_age>;

size_t migpop(const size_t & source_pop, gsl_rng * r, const double & mig_prob)
{
  if( gsl_rng_uniform(r) < mig_prob )
    {
      return ! source_pop;
    }
  return source_pop;
}

BOOST_AUTO_TEST_CASE( metapop_sugar_test1 )
{
  using poptype = KTfwd::metapop<mutation_with_age>;
  poptype pop({1000,1000});

  KTfwd::GSLrng_t<KTfwd::GSL_RNG_TAUS2> rng(0u);


  
  //Evolve for 10 generations
  std::function<double(void)> recmap = std::bind(gsl_rng_uniform,rng.get());
  std::vector<std::function<double (poptype::glist_t::const_iterator,
				    poptype::glist_t::const_iterator)> > fitness_funcs(2,
										       std::bind(KTfwd::multiplicative_diploid(),std::placeholders::_1,std::placeholders::_2,2.));
  for( unsigned generation= 0 ; generation < 10 ; ++generation )
    {
      std::vector<double> wbar = KTfwd::sample_diploid(rng.get(),
						       &pop.gametes,
						       &pop.diploids,
						       &pop.mutations,
						       &pop.Ns[0],
						       0.005,
						       std::bind(KTfwd::infsites(),rng.get(),&pop.mut_lookup,generation,
								 0.005,0.,[&rng](){return gsl_rng_uniform(rng.get());},[](){return 0.;},[](){return 0.;}),
						       std::bind(KTfwd::genetics101(),std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,
								 std::ref(pop.neutral),std::ref(pop.selected),
								 &pop.gametes,
								 0., //no rec
								 rng.get(),
								 recmap),
						       std::bind(KTfwd::insert_at_end<poptype::mutation_t,poptype::mlist_t>,std::placeholders::_1,std::placeholders::_2),
						       std::bind(KTfwd::insert_at_end<poptype::gamete_t,poptype::glist_t>,std::placeholders::_1,std::placeholders::_2),
						       fitness_funcs,
						       std::bind(KTfwd::mutation_remover(),std::placeholders::_1,0,4000),
						       std::bind(migpop,std::placeholders::_1,rng.get(),0.001)
						       );
      KTfwd::remove_fixed_lost(&pop.mutations,&pop.fixations,&pop.fixation_times,&pop.mut_lookup,generation,4000);
    }
  //attempt to copy
  KTfwd::serialize s;
  s(pop,mwriter());
  poptype pop2({0,0});
  KTfwd::deserialize()(pop2,s,mreader());

  BOOST_REQUIRE(pop.mutations.size() == pop2.mutations.size());
  BOOST_REQUIRE(pop.gametes.size() == pop2.gametes.size());
  BOOST_REQUIRE(pop.diploids.size() == pop2.diploids.size());

  //Compare the mutations
  for( auto m1 = pop.mutations.begin(),m2 = pop2.mutations.begin() ; m1 != pop.mutations.end() ; ++m1,++m2 )
    {
      BOOST_CHECK_EQUAL( m1->pos, m2->pos );
      BOOST_CHECK_EQUAL( m1->n, m2->n );
    }
  
  //Compare the gametes
  for( auto gpop1 = pop.gametes.begin(),gpop2 = pop2.gametes.begin() ; gpop1 != pop.gametes.end() ; ++gpop1,++gpop2 )
    {
      for( auto m1 = gpop1->mutations.begin(),m2=gpop2->mutations.begin() ; m1 != gpop1->mutations.end() ; ++m1,++m2 )
	{
	  BOOST_CHECK( m1 != m2 );
	  BOOST_CHECK( std::distance(pop.mutations.begin(),*m1) == std::distance(pop2.mutations.begin(),*m2) );
	  BOOST_CHECK_EQUAL( (*m1)->pos, (*m2)->pos );
	  BOOST_CHECK_EQUAL( (*m1)->n, (*m2)->n ); 
	}
    }
  
  //Compare the diploids
  auto gpop1 = pop.gametes.begin(), gpop2 = pop2.gametes.begin();
  for( auto dpop1 = pop.diploids.begin(),dpop2 = pop2.diploids.begin() ; dpop1 != pop.diploids.end() ; ++dpop1,++dpop2 )//,++gpop1,++gpop2 )
    {
      for( auto d1 = dpop1->begin(),d2 = dpop2->begin() ; d1 != dpop1->end() ; ++d1,++d2 )
	{
	  BOOST_CHECK(d1->first != d2->first);
	  BOOST_CHECK(d1->second != d2->second);
	  BOOST_CHECK( std::distance( gpop1,d1->first ) == std::distance( gpop2,d2->first ) );
	  BOOST_CHECK( std::distance( gpop1,d1->second ) == std::distance( gpop2,d2->second ) );
	  for( auto m1 = d1->first->mutations.begin(),m2=d2->first->mutations.begin() ; m1 != d1->first->mutations.end() ; ++m1,++m2 )
	    {
	      BOOST_CHECK( m1 != m2 );
	      BOOST_CHECK( std::distance(pop.mutations.begin(),*m1) == std::distance(pop2.mutations.begin(),*m2) );
	      BOOST_CHECK_EQUAL( (*m1)->pos, (*m2)->pos );
	      BOOST_CHECK_EQUAL( (*m1)->n, (*m2)->n ); 
	    }
	  for( auto m1 = d1->second->mutations.begin(),m2=d2->second->mutations.begin() ; m1 != d1->second->mutations.end() ; ++m1,++m2 )
	    {
	      BOOST_CHECK( m1 != m2 );
	      BOOST_CHECK( std::distance(pop.mutations.begin(),*m1) == std::distance(pop2.mutations.begin(),*m2) );
	      BOOST_CHECK_EQUAL( (*m1)->pos, (*m2)->pos );
	      BOOST_CHECK_EQUAL( (*m1)->n, (*m2)->n ); 
	    }
	}
    }
}


BOOST_AUTO_TEST_CASE( metapop_sugar_copy_construct_test )
{
  using poptype = KTfwd::metapop_serialized<mutation_with_age,KTfwd::mutation_writer,KTfwd::mutation_reader<KTfwd::popgenmut>>;
  poptype pop({1000,1000});

  KTfwd::GSLrng_t<KTfwd::GSL_RNG_TAUS2> rng(0u);

  //Evolve for 10 generations
  std::function<double(void)> recmap = std::bind(gsl_rng_uniform,rng.get());
  std::vector<std::function<double (poptype::glist_t::const_iterator,
				    poptype::glist_t::const_iterator)> > fitness_funcs(2,
										       std::bind(KTfwd::multiplicative_diploid(),std::placeholders::_1,std::placeholders::_2,2.));
  for( unsigned generation= 0 ; generation < 10 ; ++generation )
    {
      std::vector<double> wbar = KTfwd::sample_diploid(rng.get(),
						       &pop.gametes,
						       &pop.diploids,
						       &pop.mutations,
						       &pop.Ns[0],
						       0.005,
						       std::bind(KTfwd::infsites(),rng.get(),&pop.mut_lookup,generation,
								 0.005,0.,[&rng](){return gsl_rng_uniform(rng.get());},[](){return 0.;},[](){return 0.;}),
						       std::bind(KTfwd::genetics101(),std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,
								 std::ref(pop.neutral),std::ref(pop.selected),
								 &pop.gametes,
								 0., //no rec
								 rng.get(),
								 recmap),
						       std::bind(KTfwd::insert_at_end<poptype::mutation_t,poptype::mlist_t>,std::placeholders::_1,std::placeholders::_2),
						       std::bind(KTfwd::insert_at_end<poptype::gamete_t,poptype::glist_t>,std::placeholders::_1,std::placeholders::_2),
						       fitness_funcs,
						       std::bind(KTfwd::mutation_remover(),std::placeholders::_1,0,4000),
						       std::bind(migpop,std::placeholders::_1,rng.get(),0.001)
						       );
      KTfwd::remove_fixed_lost(&pop.mutations,&pop.fixations,&pop.fixation_times,&pop.mut_lookup,generation,4000);
    }
  //attempt to copy construct
  poptype pop2(pop);

  BOOST_REQUIRE(pop.mutations.size() == pop2.mutations.size());
  BOOST_REQUIRE(pop.gametes.size() == pop2.gametes.size());
  BOOST_REQUIRE(pop.diploids.size() == pop2.diploids.size());

  //Compare the mutations
  for( auto m1 = pop.mutations.begin(),m2 = pop2.mutations.begin() ; m1 != pop.mutations.end() ; ++m1,++m2 )
    {
      BOOST_CHECK_EQUAL( m1->pos, m2->pos );
      BOOST_CHECK_EQUAL( m1->n, m2->n );
    }
  
  //Compare the gametes
  for( auto gpop1 = pop.gametes.begin(),gpop2 = pop2.gametes.begin() ; gpop1 != pop.gametes.end() ; ++gpop1,++gpop2 )
    {
      for( auto m1 = gpop1->mutations.begin(),m2=gpop2->mutations.begin() ; m1 != gpop1->mutations.end() ; ++m1,++m2 )
	{
	  BOOST_CHECK( m1 != m2 );
	  BOOST_CHECK( std::distance(pop.mutations.begin(),*m1) == std::distance(pop2.mutations.begin(),*m2) );
	  BOOST_CHECK_EQUAL( (*m1)->pos, (*m2)->pos );
	  BOOST_CHECK_EQUAL( (*m1)->n, (*m2)->n ); 
	}
    }
  
  //Compare the diploids
  auto gpop1 = pop.gametes.begin(), gpop2 = pop2.gametes.begin();
  for( auto dpop1 = pop.diploids.begin(),dpop2 = pop2.diploids.begin() ; dpop1 != pop.diploids.end() ; ++dpop1,++dpop2,++gpop1,++gpop2 )
    {
      for( auto d1 = dpop1->begin(),d2 = dpop2->begin() ; d1 != dpop1->end() ; ++d1,++d2 )
	{
	  BOOST_CHECK(d1->first != d2->first);
	  BOOST_CHECK(d1->second != d2->second);
	  BOOST_CHECK( std::distance( gpop1,d1->first ) == std::distance( gpop2,d2->first ) );
	  BOOST_CHECK( std::distance( gpop1,d1->second ) == std::distance( gpop2,d2->second ) );
	  for( auto m1 = d1->first->mutations.begin(),m2=d2->first->mutations.begin() ; m1 != d1->first->mutations.end() ; ++m1,++m2 )
	    {
	      BOOST_CHECK( m1 != m2 );
	      BOOST_CHECK( std::distance(pop.mutations.begin(),*m1) == std::distance(pop2.mutations.begin(),*m2) );
	      BOOST_CHECK_EQUAL( (*m1)->pos, (*m2)->pos );
	      BOOST_CHECK_EQUAL( (*m1)->n, (*m2)->n ); 
	    }
	  for( auto m1 = d1->second->mutations.begin(),m2=d2->second->mutations.begin() ; m1 != d1->second->mutations.end() ; ++m1,++m2 )
	    {
	      BOOST_CHECK( m1 != m2 );
	      BOOST_CHECK( std::distance(pop.mutations.begin(),*m1) == std::distance(pop2.mutations.begin(),*m2) );
	      BOOST_CHECK_EQUAL( (*m1)->pos, (*m2)->pos );
	      BOOST_CHECK_EQUAL( (*m1)->n, (*m2)->n ); 
	    }
	}
    }
}

BOOST_AUTO_TEST_CASE( metapop_sugar_assign_test )
{
  using poptype = KTfwd::metapop_serialized<mutation_with_age,KTfwd::mutation_writer,KTfwd::mutation_reader<KTfwd::popgenmut>>;
  poptype pop({1000,1000});

  KTfwd::GSLrng_t<KTfwd::GSL_RNG_TAUS2> rng(0u);

  //Evolve for 10 generations
  std::function<double(void)> recmap = std::bind(gsl_rng_uniform,rng.get());
  std::vector<std::function<double (poptype::glist_t::const_iterator,
				    poptype::glist_t::const_iterator)> > fitness_funcs(2,
										       std::bind(KTfwd::multiplicative_diploid(),std::placeholders::_1,std::placeholders::_2,2.));
  for( unsigned generation= 0 ; generation < 10 ; ++generation )
    {
      std::vector<double> wbar = KTfwd::sample_diploid(rng.get(),
						       &pop.gametes,
						       &pop.diploids,
						       &pop.mutations,
						       &pop.Ns[0],
						       0.005,
						       std::bind(KTfwd::infsites(),rng.get(),&pop.mut_lookup,generation,
								 0.005,0.,[&rng](){return gsl_rng_uniform(rng.get());},[](){return 0.;},[](){return 0.;}),
						       std::bind(KTfwd::genetics101(),std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,
								 std::ref(pop.neutral),std::ref(pop.selected),
								 &pop.gametes,
								 0., //no rec
								 rng.get(),
								 recmap),
						       std::bind(KTfwd::insert_at_end<poptype::mutation_t,poptype::mlist_t>,std::placeholders::_1,std::placeholders::_2),
						       std::bind(KTfwd::insert_at_end<poptype::gamete_t,poptype::glist_t>,std::placeholders::_1,std::placeholders::_2),
						       fitness_funcs,
						       std::bind(KTfwd::mutation_remover(),std::placeholders::_1,0,4000),
						       std::bind(migpop,std::placeholders::_1,rng.get(),0.001)
						       );
      KTfwd::remove_fixed_lost(&pop.mutations,&pop.fixations,&pop.fixation_times,&pop.mut_lookup,generation,4000);
    }
  //attempt to copy construct
  poptype pop2 = pop;

  BOOST_REQUIRE(pop.mutations.size() == pop2.mutations.size());
  BOOST_REQUIRE(pop.gametes.size() == pop2.gametes.size());
  BOOST_REQUIRE(pop.diploids.size() == pop2.diploids.size());

  //Compare the mutations
  for( auto m1 = pop.mutations.begin(),m2 = pop2.mutations.begin() ; m1 != pop.mutations.end() ; ++m1,++m2 )
    {
      BOOST_CHECK_EQUAL( m1->pos, m2->pos );
      BOOST_CHECK_EQUAL( m1->n, m2->n );
    }
  
  //Compare the gametes
  for( auto gpop1 = pop.gametes.begin(),gpop2 = pop2.gametes.begin() ; gpop1 != pop.gametes.end() ; ++gpop1,++gpop2 )
    {
      for( auto m1 = gpop1->mutations.begin(),m2=gpop2->mutations.begin() ; m1 != gpop1->mutations.end() ; ++m1,++m2 )
	{
	  BOOST_CHECK( m1 != m2 );
	  BOOST_CHECK( std::distance(pop.mutations.begin(),*m1) == std::distance(pop2.mutations.begin(),*m2) );
	  BOOST_CHECK_EQUAL( (*m1)->pos, (*m2)->pos );
	  BOOST_CHECK_EQUAL( (*m1)->n, (*m2)->n ); 
	}
    }
  
  //Compare the diploids
  auto gpop1 = pop.gametes.begin(), gpop2 = pop2.gametes.begin();
  for( auto dpop1 = pop.diploids.begin(),dpop2 = pop2.diploids.begin() ; dpop1 != pop.diploids.end() ; ++dpop1,++dpop2 )//,++gpop1,++gpop2 )
    {
      for( auto d1 = dpop1->begin(),d2 = dpop2->begin() ; d1 != dpop1->end() ; ++d1,++d2 )
	{
	  BOOST_CHECK(d1->first != d2->first);
	  BOOST_CHECK(d1->second != d2->second);
	  BOOST_CHECK( std::distance( gpop1,d1->first ) == std::distance( gpop2,d2->first ) );
	  BOOST_CHECK( std::distance( gpop1,d1->second ) == std::distance( gpop2,d2->second ) );
	  for( auto m1 = d1->first->mutations.begin(),m2=d2->first->mutations.begin() ; m1 != d1->first->mutations.end() ; ++m1,++m2 )
	    {
	      BOOST_CHECK( m1 != m2 );
	      BOOST_CHECK( std::distance(pop.mutations.begin(),*m1) == std::distance(pop2.mutations.begin(),*m2) );
	      BOOST_CHECK_EQUAL( (*m1)->pos, (*m2)->pos );
	      BOOST_CHECK_EQUAL( (*m1)->n, (*m2)->n ); 
	    }
	  for( auto m1 = d1->second->mutations.begin(),m2=d2->second->mutations.begin() ; m1 != d1->second->mutations.end() ; ++m1,++m2 )
	    {
	      BOOST_CHECK( m1 != m2 );
	      BOOST_CHECK( std::distance(pop.mutations.begin(),*m1) == std::distance(pop2.mutations.begin(),*m2) );
	      BOOST_CHECK_EQUAL( (*m1)->pos, (*m2)->pos );
	      BOOST_CHECK_EQUAL( (*m1)->n, (*m2)->n ); 
	    }
	}
    }
}
