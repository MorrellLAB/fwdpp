/*!
  \file test_sugar_add_mutation.cc

  \brief test KTfwd::add_mutation
*/

#define BOOST_TEST_MODULE test_sugar_add_mutation
#define BOOST_TEST_DYN_LINK 

#include <config.h>
#include <boost/test/unit_test.hpp>
#include <fwdpp/diploid.hh>
#include <fwdpp/sugar/GSLrng_t.hpp>
#include <fwdpp/sugar/singlepop.hpp>
#include <fwdpp/sugar/metapop.hpp>
#include <fwdpp/sugar/multiloc.hpp>
#include <fwdpp/sugar/infsites.hpp>
#include <fwdpp/sugar/serialization.hpp>
#include <fwdpp/sugar/add_mutation.hpp>

using mutation_t = KTfwd::popgenmut;
using mwriter = KTfwd::mutation_writer;
using mreader = KTfwd::mutation_reader<mutation_t>;
using singlepop_t = KTfwd::singlepop<mutation_t>;
using metapop_t = KTfwd::metapop<mutation_t>;
using multiloc_t = KTfwd::multiloc<mutation_t>;

/*
  First, unit tests for the addition of a single new mutation.

  Main use case envisioned for such functions is adding in a new mutation 
  during a simulation.
*/

BOOST_AUTO_TEST_CASE( test_add_mutation )
{
  singlepop_t pop(1000);
  KTfwd::add_mutation(pop,
		      //individuals where we want to place the mutation
		      {0,1,3,5,7,9},
		      /*
			gametes in each individual: 0 = .first, 1 = .second, 2 = .first and .second
			Thus, there should be 1+1+1+2+2+1=8 copies of the mutation in the population
		      */
		      {0,1,0,2,2,0},
		      //Parameters to pass on to create a new mutation
		      0.1,-0.1,1,0);
  BOOST_REQUIRE_EQUAL(KTfwd::check_sum(pop.gametes,2000),true);
  BOOST_REQUIRE_EQUAL(pop.gametes.size(),2); 
  BOOST_REQUIRE_EQUAL(pop.mutations.size(),1);
  BOOST_REQUIRE_EQUAL(pop.mcounts.size(),1);
  BOOST_REQUIRE_EQUAL(pop.mcounts[0],8);
  BOOST_REQUIRE_EQUAL(pop.mutations[0].neutral,false);

  for( auto i : {0,3,9} ) //should have mutation on first gamete only
    {
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i].first].smutations.size(),1 );
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i].second].smutations.size(),0 );
    }
  for( auto i : {1} ) //should have mutation on second gamete only
    {
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i].first].smutations.size(),0 );
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i].second].smutations.size(),1 );
    }
  for(auto i : {5,7}) // should have mutations on both gametes
    {
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i].first].smutations.size(),1 );
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i].second].smutations.size(),1 );
    }
}

BOOST_AUTO_TEST_CASE( test_add_mutation_from_object )
{
  singlepop_t pop(1000);
  mutation_t m(0.1,-0.1,1,0);
  KTfwd::add_mutation(pop,
		      //individuals where we want to place the mutation
		      {0,1,3,5,7,9},
		      /*
			gametes in each individual: 0 = .first, 1 = .second, 2 = .first and .second
			Thus, there should be 1+1+1+2+2+1=8 copies of the mutation in the population
		      */
		      {0,1,0,2,2,0},
		      //move it right into place:
		      std::move(m));
  BOOST_REQUIRE_EQUAL(KTfwd::check_sum(pop.gametes,2000),true);
  BOOST_REQUIRE_EQUAL(pop.gametes.size(),2); 
  BOOST_REQUIRE_EQUAL(pop.mutations.size(),1);
  BOOST_REQUIRE_EQUAL(pop.mcounts.size(),1);
  BOOST_REQUIRE_EQUAL(pop.mcounts[0],8);
  BOOST_REQUIRE_EQUAL(pop.mutations[0].neutral,false);

  for( auto i : {0,3,9} ) //should have mutation on first gamete only
    {
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i].first].smutations.size(),1 );
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i].second].smutations.size(),0 );
    }
  for( auto i : {1} ) //should have mutation on second gamete only
    {
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i].first].smutations.size(),0 );
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i].second].smutations.size(),1 );
    }
  for(auto i : {5,7}) // should have mutations on both gametes
    {
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i].first].smutations.size(),1 );
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i].second].smutations.size(),1 );
    }
}

BOOST_AUTO_TEST_CASE( test_add_mutation_metapop )
{
  metapop_t pop({1000,1000});
  const std::size_t DEME = 1; //we're gonna add the mutation into the second deme
  /*
    Note: for meta-pops, function takes vectors and vectors of vectors,
    so use via an initializer list has an extra bracket around it...
  */
  KTfwd::add_mutation(pop,
		      //deme index...
		      {DEME},
		      //individuals where we want to place the mutation
		      {{{0,1,3,5,7,9}}},
		      /*
			gametes in each individual: 0 = .first, 1 = .second, 2 = .first and .second
			Thus, there should be 1+1+1+2+2+1=8 copies of the mutation in the population
		      */
		      {{{0,1,0,2,2,0}}},
		      //For fun, pass in new mutation as a temporary
		      mutation_t(0.1,-0.1,1,0));
  BOOST_REQUIRE_EQUAL(KTfwd::check_sum(pop.gametes,4000),true);
  BOOST_REQUIRE_EQUAL(pop.gametes.size(),2); 
  BOOST_REQUIRE_EQUAL(pop.mutations.size(),1);
  BOOST_REQUIRE_EQUAL(pop.mcounts.size(),1);
  BOOST_REQUIRE_EQUAL(pop.mcounts[0],8);
  BOOST_REQUIRE_EQUAL(pop.mutations[0].neutral,false);

  for( auto i : {0,3,9} ) //should have mutation on first gamete only
    {
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[DEME][i].first].smutations.size(),1 );
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[DEME][i].second].smutations.size(),0 );
    }
  for( auto i : {1} ) //should have mutation on second gamete only
    {
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[DEME][i].first].smutations.size(),0 );
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[DEME][i].second].smutations.size(),1 );
    }
  for(auto i : {5,7}) // should have mutations on both gametes
    {
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[DEME][i].first].smutations.size(),1 );
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[DEME][i].second].smutations.size(),1 );
    }
}

BOOST_AUTO_TEST_CASE( test_add_mutation_multiloc )
{
  multiloc_t pop(1000,10); //10-locus system
  const std::size_t LOCUS=7; //we'll add mutations into the 8-th locus
  KTfwd::add_mutation(pop,
		      //locus index...
		      LOCUS,
		      //individuals where we want to place the mutation
		      {0,1,3,5,7,9},
		      /*
			gametes in each individual: 0 = .first, 1 = .second, 2 = .first and .second
			Thus, there should be 1+1+1+2+2+1=8 copies of the mutation in the population
		      */
		      {0,1,0,2,2,0},
		      //params for new mutation
		      0.1,-0.1,1,0);
  BOOST_REQUIRE_EQUAL(KTfwd::check_sum(pop.gametes,10*2000),true);
  BOOST_REQUIRE_EQUAL(pop.gametes.size(),2); 
  BOOST_REQUIRE_EQUAL(pop.mutations.size(),1);
  BOOST_REQUIRE_EQUAL(pop.mcounts.size(),1);
  BOOST_REQUIRE_EQUAL(pop.mcounts[0],8);
  BOOST_REQUIRE_EQUAL(pop.mutations[0].neutral,false);
  for( auto i : {0,3,9} ) //should have mutation on first gamete only
    {
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i][LOCUS].first].smutations.size(),1 );
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i][LOCUS].second].smutations.size(),0 );
    }
  for( auto i : {1} ) //should have mutation on second gamete only
    {
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i][LOCUS].first].smutations.size(),0 );
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i][LOCUS].second].smutations.size(),1 );
    }
  for(auto i : {5,7}) // should have mutations on both gametes
    {
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i][LOCUS].first].smutations.size(),1 );
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i][LOCUS].second].smutations.size(),1 );
    }
}

/*
  Second, unit tests for the addition of multuple mutations to a common set 
  of gametes.

  Main use case envisioned for such functions is initializing a simulation
  from a pre-existing set of data, perhaps existing in a file somewhere.
*/

BOOST_AUTO_TEST_CASE( test_add_mutations_singlepop )
/*
  Note: this unit tests updates the same diploids at different steps.

  This is done so that the test is more complex than the intended use case,
  which is to collect mutation info and what individuals contain those mutations
  from a file (for example), and then apply this function based on sets of mutations
  w.r.to what diploids they are found in.
*/
{
  singlepop_t pop(1000);
  //Add three new mutations to this population object
  pop.mutations.emplace_back(0.1,0.0,0.0,0); //Neutral at position 0.1
  pop.mutations.emplace_back(0.2,-0.1,0.0,0); //Selected, deleterious, recessive at position 0.2
  pop.mutations.emplace_back(0.3,0.1,1.0,0); //Selected, beneficial, h=1 at position 0.3,
  pop.mcounts.resize(3,0); //got make sure mcounts matches up in size.  We init to zero b/c add_mutations will do the work for us.
  
  //We are going to add mutations 0 and 2 to individuals 9, 10, and 23.
  //These individuals will be heterozygotes, which these mutations
  //added to the first gamete in each diploid.
  KTfwd::add_mutations(pop,
		       {9,10,23},
		       {0,0,0},
		       {0,2});

  //Make sure what we expect is what we got
  BOOST_REQUIRE_EQUAL(pop.gametes.size(),2);
  BOOST_REQUIRE_EQUAL(pop.gametes[0].n,1997);
  BOOST_REQUIRE_EQUAL(pop.gametes[1].n,3);
  for( auto i : {0,2} )
    {
      BOOST_REQUIRE_EQUAL(pop.mcounts[i],3);
    }
  
  
  //Mutation 1 will be added to the first 100 individuals,
  //who will all be homozygous.
  std::vector<std::size_t> first100(100);
  std::size_t i=0;
  std::generate(first100.begin(),first100.end(),
		[&i](){return i++;});
  KTfwd::add_mutations(pop,
		       first100,
		       std::vector<short>(100,2),
		       {1});
  /*
    This next test is tricky:
    1. The gametes created above will now go extinct b/c they get a new variant added, 
    thus creating a new gamete
    2. Then, there is a new gamete created at high frequency
   */
  BOOST_REQUIRE_EQUAL(pop.gametes.size(),4);
  BOOST_REQUIRE_EQUAL(pop.gametes[0].n,1800); //should not test further than this, as it depends on internal details subject to change!

  //check those first few individuals
  for(auto i: {9,10,23})
    {
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i].first].mutations.size(),1 );
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i].first].smutations.size(),2 );
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i].second].mutations.size(),0 );
      BOOST_REQUIRE_EQUAL( pop.gametes[pop.diploids[i].second].smutations.size(),1 );
    }
}
