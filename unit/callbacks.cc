#define BOOST_TEST_MODULE callbackTest
#define BOOST_TEST_DYN_LINK 

//This is really an API check

#include <config.h>
#include <fwdpp/extensions/callbacks.hpp>
#include <boost/test/unit_test.hpp>
#include <unistd.h>
#include <iterator>
#include <functional>
#include <list>
#include <iostream>

gsl_rng * r =  gsl_rng_alloc(gsl_rng_ranlxs2);

BOOST_AUTO_TEST_CASE(constant)
{
  gsl_rng_set(r,101);
  KTfwd::extensions::constant c(1);
}

BOOST_AUTO_TEST_CASE(exponential)
{
  gsl_rng_set(r,101);
  KTfwd::extensions::exponential e(1);
}

BOOST_AUTO_TEST_CASE(uniform)
{
  gsl_rng_set(r,101);
  KTfwd::extensions::uniform e(0,1);
}

BOOST_AUTO_TEST_CASE(beta)
{
  gsl_rng_set(r,101);
  KTfwd::extensions::beta b(1,2,1.0);
}

BOOST_AUTO_TEST_CASE(Gamma)
{
  gsl_rng_set(r,101);
  KTfwd::extensions::gamma b(1,2);
}

BOOST_AUTO_TEST_CASE(gaussian)
{
  gsl_rng_set(r,101);
  KTfwd::extensions::gaussian e(1);
}

BOOST_AUTO_TEST_CASE(shmodel)
{
  gsl_rng_set(r,101);
  //default construct
  KTfwd::extensions::shmodel x;
  //These both eval to the "right thing";
  x.s = std::bind(KTfwd::extensions::gamma(1,2),std::placeholders::_1);
  x.h = KTfwd::extensions::gamma(1,2);

  auto __s = x.s(r);
  auto __h = x.h(r);
}

BOOST_AUTO_TEST_CASE(shmodel2)
{
  gsl_rng_set(r,101);
  //Construct and consume the input arguments
  KTfwd::extensions::shmodel x(std::bind(KTfwd::extensions::gamma(1,2),std::placeholders::_1),
			       KTfwd::extensions::gamma(1,2));
  auto __s = x.s(r);
  auto __h = x.h(r);
}

BOOST_AUTO_TEST_CASE( point_mass_test1 )
{
  gsl_rng_set(r,101);
  auto x = KTfwd::extensions::uniform(-1.,-1.);
  auto y = x(r);
  BOOST_REQUIRE_EQUAL(y,-1.0);
}

//The callbacks can throw exceptions if their parameters aren't valid

BOOST_AUTO_TEST_CASE( callback_exceptions )
{
  {
    //inf
    BOOST_REQUIRE_THROW( KTfwd::extensions::constant(1./0.), 
			 std::runtime_error
			 );
  }
  
  {
    //nan
    BOOST_REQUIRE_THROW( KTfwd::extensions::constant(std::nan("")), 
			 std::runtime_error
			 );
  }

  {
    //first arg not finite
    BOOST_REQUIRE_THROW( KTfwd::extensions::uniform(1./0., 1.),
			 std::runtime_error
			 );
  }

  {
    //2nd arg not finite
    BOOST_REQUIRE_THROW( KTfwd::extensions::uniform(1., 1./0.),
			 std::runtime_error
			 );
  }

  {
    //min > max
    BOOST_REQUIRE_THROW( KTfwd::extensions::uniform(1., 0.99),
			 std::runtime_error
			 );
  }

  {
    //a not finite
    BOOST_REQUIRE_THROW( KTfwd::extensions::beta(std::nan(""),1.),
			 std::runtime_error
			 );
  }

  {
    //b not finite
    BOOST_REQUIRE_THROW( KTfwd::extensions::beta(1.,std::nan("")),
			 std::runtime_error
			 );
  }

  {
    //f not finite
    BOOST_REQUIRE_THROW( KTfwd::extensions::beta(1.,1.,std::nan("")),
			 std::runtime_error
			 );
  }

  {
    //a <= 0.
    BOOST_REQUIRE_THROW( KTfwd::extensions::beta(0.,1.),
			 std::runtime_error
			 );
  }

  {
    //b <= 0.
    BOOST_REQUIRE_THROW( KTfwd::extensions::beta(1.,0.),
			 std::runtime_error
			 );
  }

  {
    //f <= 0.
    BOOST_REQUIRE_THROW( KTfwd::extensions::beta(1.,1.,0.),
			 std::runtime_error
			 );
  }

  {
    //sd = 0
    BOOST_REQUIRE_THROW( KTfwd::extensions::gaussian(0.),
			 std::runtime_error
			 );
  }
  
  {
    //sd < 0
    BOOST_REQUIRE_THROW( KTfwd::extensions::gaussian(-1e-6),
			 std::runtime_error
			 );
  }

  {
    //sd not finite
    BOOST_REQUIRE_THROW( KTfwd::extensions::gaussian(1./0.),
			 std::runtime_error
			 );
  }

  {
    //mean not finite
    BOOST_REQUIRE_THROW( KTfwd::extensions::gamma(1./0.,1.),
			 std::runtime_error
			 );
  }

  {
    //shape not finite
    BOOST_REQUIRE_THROW( KTfwd::extensions::gamma(1.0,1./0.),
			 std::runtime_error
			 );
  }

  {
    //!(shape>0)
    BOOST_REQUIRE_THROW( KTfwd::extensions::gamma(1.0,0.),
			 std::runtime_error
			 );
  }
}
