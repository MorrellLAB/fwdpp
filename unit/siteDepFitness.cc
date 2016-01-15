/*!
  \file siteDepFitness.cc
  \brief Unit tests of KTfwd::site_dependent_fitness
  \ingroup unit
*/

#define BOOST_TEST_MODULE siteDepFitness
#define BOOST_TEST_DYN_LINK 

#include <config.h>
#include <unistd.h>
#include <iterator>
#include <functional>
#include <iostream>
#include <cmath>
#include <vector>
#include <fwdpp/diploid.hh>
#include <fwdpp/sugar/singlepop.hpp>
#include <boost/test/unit_test.hpp>
#include <custom_dip.hpp>

using mut = KTfwd::mutation;
using gtype = KTfwd::gamete;
using mvector = std::vector<mut>;
using gvector = std::vector<gtype>;

/*
  This test creates a situation
  where gamete1 has a selected mutation and gamete2 does not.
  If issue #8 were to cause fitness to be mis-calculated,
  then this test will fail.

  However, it does not.  Even with the bug, the remaining bit of the function
  gets the calculation right.  Yay!
*/
BOOST_AUTO_TEST_CASE( simple_multiplicative1 )
{
  gtype g1(1),g2(1);
  mvector mutations;

  //add mutation at position 0.1, s=0.1,n=1,dominance=0.5 (but we won't use the dominance...)
  mutations.emplace_back(0.1,0.1,0.5);
  KTfwd::fwdpp_internal::add_new_mutation(0,mutations,g1);
  BOOST_CHECK_EQUAL(g1.smutations.size(),1);
  
  gvector g{g1,g2};
  double w = KTfwd::site_dependent_fitness()(g[0],g[1],mutations,
					     [&](double & fitness,const mut & __mut)
					     {
					       fitness *= std::pow(1. + __mut.s,2.);
					     },
					     [&](double & fitness,const mut & __mut)
					     {
					       fitness *= (1. + __mut.s);
					     },
					     1.);
  
  BOOST_CHECK_EQUAL(w,1.1);
}

/*
  g2 has it, g1 does not
*/
BOOST_AUTO_TEST_CASE( simple_multiplicative2 )
{
  gtype g1(1),g2(1);
  mvector mutations;

  //add mutation at position 0.1, s=0.1,n=1,dominance=0.5 (but we won't use the dominance...)
  mutations.emplace_back( 0.1,0.1,0.5 );
  KTfwd::fwdpp_internal::add_new_mutation(0,mutations,g2);
  BOOST_CHECK_EQUAL(g1.smutations.size(),0);
  BOOST_CHECK_EQUAL(g2.smutations.size(),1);

  gvector g{g1,g2};

  double w = KTfwd::site_dependent_fitness()(g[0],g[1],mutations,
					     [&](double & fitness,const mut & __mut)
					     {
					       fitness *= std::pow(1. + __mut.s,2.);
					     },
					     [&](double & fitness,const mut & __mut)
					     {
					       fitness *= (1. + __mut.s);
					     },
					     1.);
  BOOST_CHECK_EQUAL(w,1.1);
}

/*
  Both have it
*/
BOOST_AUTO_TEST_CASE( simple_multiplicative3 )
{
  gtype g1(1),g2(1);
  mvector mutations;

  //add mutation at position 0.1, s=0.1,n=1,dominance=0.5 (but we won't use the dominance...)
  mutations.emplace_back(0.1,0.1,0.5 );
  KTfwd::fwdpp_internal::add_new_mutation(0,mutations,g2);
  KTfwd::fwdpp_internal::add_new_mutation(0,mutations,g1);
  BOOST_CHECK_EQUAL(g1.smutations.size(),1);
  BOOST_CHECK_EQUAL(g2.smutations.size(),1);

  gvector g{g1,g2};

  double w = KTfwd::site_dependent_fitness()(g[0],g[1],mutations,
					     [&](double & fitness,const mut & __mut)
					     {
					       fitness *= std::pow(1. + __mut.s,2.);
					     },
					     [&](double & fitness,const mut & __mut)
					     {
					       fitness *= (1. + __mut.s);
					     },
					     1.);
  BOOST_CHECK_EQUAL(w,1.1*1.1);
}

/*
  Now, g1 has 2 mutations
*/
BOOST_AUTO_TEST_CASE( simple_multiplicative4 )
{
  gtype g1(1),g2(1);
  mvector mutations;

  //add mutation at position 0.1, s=0.1,n=1,dominance=0.5 (but we won't use the dominance...)
  mutations.emplace_back(0.1,0.1,0.5 );
  KTfwd::fwdpp_internal::add_new_mutation(0,mutations,g1);
  mutations.emplace_back(0.2,0.1,0.5);
  KTfwd::fwdpp_internal::add_new_mutation(1,mutations,g1);
  BOOST_CHECK_EQUAL(g1.smutations.size(),2);

  gvector g{g1,g2};

  double w = KTfwd::site_dependent_fitness()(g[0],g[1],mutations,
					     [&](double & fitness,const mut & __mut)
					     {
					       fitness *= std::pow(1. + __mut.s,2.);
					     },
					     [&](double & fitness,const mut & __mut)
					     {
					       fitness *= (1. + __mut.s);
					     },
					     1.);
  BOOST_CHECK_EQUAL(w,1.1*1.1);
}

BOOST_AUTO_TEST_CASE( simple_additive_1 )
{
  gtype g1(1),g2(1);
  mvector mutations;

  //add mutation at position 0.1, s=0.1,n=1,dominance=1.0
  mutations.emplace_back(0.1,0.1,1 );
  KTfwd::fwdpp_internal::add_new_mutation(0,mutations,g1);
  mutations.emplace_back(0.2,0.1,1);
  KTfwd::fwdpp_internal::add_new_mutation(1,mutations,g1);
  BOOST_CHECK_EQUAL(g1.smutations.size(),2);

  gvector g{g1,g2};

  double w = KTfwd::additive_diploid()(g[0],g[1],mutations);
  BOOST_CHECK_EQUAL(w,1.2);  
}

/*
  API checks on fitness policies.

  Below, we test the ability to assign bound fitness models to variables
  and then reassign them.

  We make use of types defined by population objects from the "sugar" layer.
*/

BOOST_AUTO_TEST_CASE( reassign_test_1 )
{
  //Tests using a "standard" diploid type
  using poptype = KTfwd::singlepop<mut>;
  
  {
    //Test reassignment of the SAME fitness model type
    //Multiplicative model first
    
    //assign a fitness model with default scaling = 1.
    poptype::fitness_t dipfit = std::bind(KTfwd::multiplicative_diploid(),
					  std::placeholders::_1,
					  std::placeholders::_2,
					  std::placeholders::_3);
    
    //Now, reassign it with scaling = 2.
    dipfit = std::bind(KTfwd::multiplicative_diploid(),
		       std::placeholders::_1,
		       std::placeholders::_2,
		       std::placeholders::_3,
		       2.);
  }

  {
    //Test reassignment of the SAME fitness model type
    //Now the additive model
    
    //assign a fitness model with default scaling = 1.
    poptype::fitness_t dipfit = std::bind(KTfwd::additive_diploid(),
					  std::placeholders::_1,
					  std::placeholders::_2,
					  std::placeholders::_3);
    //Now, reassign it with scaling = 2.
    dipfit = std::bind(KTfwd::additive_diploid(),
		       std::placeholders::_1,
		       std::placeholders::_2,
		       std::placeholders::_3,
		       2.);
  }

  {
    //Convert a multiplicative model to an additive model
    //The application of this is a program using the library
    //that wants to decide which model to use based on parameters
    //passed in by a user.
    
    poptype::fitness_t dipfit = std::bind(KTfwd::multiplicative_diploid(),
					  std::placeholders::_1,
					  std::placeholders::_2,
					  std::placeholders::_3);

    //Now, reassign it to addtive model with scaling = 2.
    dipfit = std::bind(KTfwd::additive_diploid(),
		       std::placeholders::_1,
		       std::placeholders::_2,
		       std::placeholders::_3,
		       2.);
  }
}

BOOST_AUTO_TEST_CASE( reassign_test_2 )
{
  //Tests using a custom diploid type
  using poptype = KTfwd::singlepop<mut,diploid_t>;
  //using poptype = KTfwd::singlepop<mut>;

  {
    //Test reassignment of the SAME fitness model type
    //Multiplicative model first

    //assign a fitness model with default scaling = 1.
    poptype::fitness_t dipfit = std::bind(KTfwd::multiplicative_diploid(),
					  std::placeholders::_1,
					  std::placeholders::_2,
					  std::placeholders::_3);

    //Now, reassign it with scaling = 2.
    dipfit = std::bind(KTfwd::multiplicative_diploid(),
    		       std::placeholders::_1,
    		       std::placeholders::_2,
    		       std::placeholders::_3,
    		       2.);
  }

  {
    //Test reassignment of the SAME fitness model type
    //Now the additive model
    
    //assign a fitness model with default scaling = 1.
    poptype::fitness_t dipfit = std::bind(KTfwd::additive_diploid(),
					  std::placeholders::_1,
					  std::placeholders::_2,
					  std::placeholders::_3);
    //Now, reassign it with scaling = 2.
    dipfit = std::bind(KTfwd::additive_diploid(),
    		       std::placeholders::_1,
    		       std::placeholders::_2,
    		       std::placeholders::_3,
    		       2.);
  }

  {
    //Convert a multiplicative model to an additive model
    //The application of this is a program using the library
    //that wants to decide which model to use based on parameters
    //passed in by a user.
    
    poptype::fitness_t dipfit = std::bind(KTfwd::multiplicative_diploid(),
					  std::placeholders::_1,
					  std::placeholders::_2,
					  std::placeholders::_3);

    //Now, reassign it to addtive model with scaling = 2.
    dipfit = std::bind(KTfwd::additive_diploid(),
    		       std::placeholders::_1,
    		       std::placeholders::_2,
    		       std::placeholders::_3,
    		       2.);
  }
}
