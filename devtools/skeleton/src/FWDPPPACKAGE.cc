/*
  Some basic documentation should go here

  This is a MINIMAL .cc file.
 */
#include <fwdpp/diploid.hh>
#include <config.h>
//If you make a custom mutation type,
//you need to define it above,
//and adjust this using declaration:
using mtype = KTfwd::mutation;

/*
  The results of header checks will be found in config.h
  Basically, if boost stuff is present anad available,
  boost containers will be used.  However, that can be
  over-ridded by passing --enable-standard=yes to ./configure
*/
#if defined(HAVE_BOOST_VECTOR) && defined(HAVE_BOOST_LIST) && defined(HAVE_BOOST_UNORDERED_SET) && defined(HAVE_BOOST_POOL_ALLOC) && defined(HAVE_BOOST_HASH) && !defined(USE_STANDARD_CONTAINERS)
#include <boost/container/vector.hpp>
#include <boost/container/list.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <boost/unordered_set.hpp>
#include <boost/functional/hash.hpp>
/*
  IMPORTANT:  If you plan on writing a simulation
  using multiple threads of execution, then you MUST do 
  one of the following:
  1. replace boost::pool_allocator with std::allocator
  OR
  2. write your own thread-safe pool allocator. (If you can do this,
  put it on Github so that others can use it...)
  
  boost::pool_allocator is NOT thread-safe!!!!
 */
using mut_allocator =  boost::pool_allocator<mtype>;
using mlist = boost::container::list<mtype,mut_allocator >;
using gtype = KTfwd::gamete_base<mtype,mlist>;
using gam_allocator = boost::pool_allocator<gtype>;
using glist = boost::container::list<gtype,gam_allocator >;
using lookup_table_type = boost::unordered_set<double,boost::hash<double>,KTfwd::equal_eps >;
#else
#include <vector>
#include <list>
#include <unordered_set>
#include <functional>
using mlist = std::container<mtype>;
using gtype = KTfwd::gamete_base<mtype>;
using glist = std::list<gtype>;
using lookup_table_type = std::unordered_set<double,std::hash<double>,KTfwd::equal_eps >;
#endif

using namespace std;

int main(int argc, char ** argv)
{
}
