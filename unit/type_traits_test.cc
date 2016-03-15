/*!
  \file type_traits_test.cc
  \ingroup unit
  \brief Testing fwdpp/type_traits.hpp

  These tests make sure that the type traits 
  actually return what we expect them to.
*/

#define BOOST_TEST_MODULE type_traits_test
#define BOOST_TEST_DYN_LINK

#include <config.h>
#include <boost/test/unit_test.hpp>
#include <fwdpp/diploid.hh>
#include <fwdpp/sugar/sugar.hpp>
using mutation_t = KTfwd::popgenmut;
using singlepop_t = KTfwd::singlepop<mutation_t>;

struct trivial_custom_diploid_invalid : public KTfwd::tags::custom_diploid_t
{
};

struct trivial_custom_diploid_valid : public KTfwd::tags::custom_diploid_t
{
  using first_type = std::size_t;
  using second_type = std::size_t;
};

BOOST_AUTO_TEST_CASE( is_diploid_like_test )
{
  auto v = KTfwd::traits::is_diploid_like<std::pair<std::size_t,std::size_t> >::value;
  BOOST_REQUIRE_EQUAL(v,true);
  v = KTfwd::traits::is_custom_diploid_t<std::pair<std::size_t,std::size_t> >::value;
  BOOST_REQUIRE_EQUAL(v,false);
}

BOOST_AUTO_TEST_CASE( is_gamete_test )
{
  singlepop_t pop(100);
  auto v = KTfwd::traits::is_gamete_t<singlepop_t::gamete_t>::value;
  BOOST_REQUIRE_EQUAL(v,true);
}

BOOST_AUTO_TEST_CASE( is_custom_diploid_test )
{
  auto v = KTfwd::traits::is_custom_diploid_t<trivial_custom_diploid_invalid >::value;
  BOOST_REQUIRE_EQUAL(v,false);
  v = KTfwd::traits::is_custom_diploid_t<trivial_custom_diploid_valid >::value;
  BOOST_REQUIRE_EQUAL(v,true);
}

BOOST_AUTO_TEST_CASE( is_mmodel_test )
{
  singlepop_t pop(100);
  KTfwd::GSLrng_t<KTfwd::GSL_RNG_MT19937> r(101);
  auto mp = std::bind(KTfwd::infsites(),std::placeholders::_1,std::placeholders::_2,r.get(),std::ref(pop.mut_lookup),0,
		      0.001,0.,[&r](){return gsl_rng_uniform(r.get());},[](){return 0.;},[](){return 0.;});
  auto v = std::is_convertible<decltype(mp),KTfwd::traits::mmodel_t<singlepop_t::mcont_t> >::value;
  BOOST_REQUIRE_EQUAL(v,true);

  //This also implicitly tests correctness of traits::recycling_bin_t.  If that isn't working, this won't compile.
  v=KTfwd::traits::valid_mutation_model<decltype(mp),singlepop_t::mcont_t,singlepop_t::gcont_t>::value;
  BOOST_REQUIRE_EQUAL(v,true);
}

BOOST_AUTO_TEST_CASE( is_standard_fitness_model_test )
{
  singlepop_t pop(100);
  auto fp = std::bind(KTfwd::multiplicative_diploid(),std::placeholders::_1,std::placeholders::_2,
		      std::placeholders::_3,2.);
  auto v = std::is_convertible<decltype(fp),KTfwd::traits::fitness_fxn_t<singlepop_t::dipvector_t,singlepop_t::gcont_t,singlepop_t::mcont_t> >::value;
  BOOST_REQUIRE_EQUAL(v,true);
}

BOOST_AUTO_TEST_CASE( is_recmodel_test )
{
  singlepop_t pop(100);
  KTfwd::GSLrng_t<KTfwd::GSL_RNG_MT19937> r(101);
  auto rm = std::bind(KTfwd::poisson_xover(),r.get(),1e-2,0.,1.,
		      std::placeholders::_1,std::placeholders::_2,std::placeholders::_3);
  auto v = std::is_convertible<decltype(rm),KTfwd::traits::recmodel_t<singlepop_t::gcont_t,singlepop_t::mcont_t> >::value;
  BOOST_REQUIRE_EQUAL(v,true);
  v = KTfwd::traits::valid_rec_model<decltype(rm),singlepop_t::gamete_t,singlepop_t::mcont_t>::value;
  BOOST_REQUIRE_EQUAL(v,true);
}
