# Tutorial 1: Policies in fwdpp

[TOC]

\section TutIntro Introduction

This document is intended to be an in-depth discussion of policies and their role in implementing forward-time population genetic simulations using the C++ template library __fwdpp__.  We will first describe what policies are using standard C++ examples, and then we will get into the harder stuff.

In this document, I am implictly assuming that the library is being used to implement what I refer to as a "single-locus" simulation, by which I mean the simulation of a contiguous genomic stretch to which we apply some arbitrary model of mutation, recombination, and fitness.  It is possible to use the same machinery to implement a "multi-locus" model of non-contiguous genomic segments, each with different mutation models, recombination models, and arbitrary linkage relationships amongst them.  However, such simulations are arguably easier to impement using the multilocus API provided by the library, and I refer the users to the [documentation](@ref md_md_multiloc) for that part of the library.

An understanding of C++ fundamentals, including containers, their iterators, and how they relate to the standard algorithms, is assumed knowledge here.

\section TutGeneral Policies in C++

## Policies are everywhere
Policies are a part of every programming language.  Generally-speaking, they modify the behavior of what functions are with (or to) data.  In other words, they turn a generic function into a piece of code doing a specific task.  Let's start with the rather trivial example of sorting a vector:

~~~{.cpp}
#include <algorithm>
#include <vector>

using namespace std;

int main ( int argc, char ** argv )
{
  vector<unsigned> vu{10,9,8,7,6,5,4,3,2,1};

  sort(vu.begin(),vu.end());
}
~~~

The above example is 100\% standard C++.  But what is going on under the hood of the sort function is quite interesting.  A sort algorithm is being executed, and the values are being compared via a call to this function:

~~~{.cpp}
template <class T> struct less : binary_function <T,T,bool> {
    //algorithms can't guess return types,
    //and therefore often need this typedef
    typedef bool result_type;
bool operator() (const T& x, const T& y) const {return x<y;}
  };
~~~

## Policies are often function objects
The structure called less is a "function object'', and is the policy being employed in the sort.  Further, is a template, meaning it works on any data type for which the "less than" operator is defined, e.g., any type for which this code is valid:

~~~{.cpp}
if ( a < b )
{
}
~~~

The detail that less inherits from __std::binary\_function__ is important for how it plugs into the sort algorithm, but we'll deal with those issues later.

The way such a function object is used looks like:

~~~{.cpp}
int x = 5, y = 6;
  /*
  The less() instantiates a (temporary) object of type less<T>,
  where T = int in this case.
  the (x,y) passes those to variables to the operator() of less.
  (This is where the term
  "function object'', or functor for short, comes from.)
  */
  bool x_is_less = less()(x,y);
~~~

### Policies change behavior of algorithms

OK, so now we hopefully have a basic understanding of what a policy is and that algorithms in C++ work through policies implemented as function objects.  This lets us change the behavior of algorithms:

~~~{.cpp}
  #include <functional> //need this header for std::greater<T>
  //Sort in descending order (biggest values @ front of vu after sort)
  sort( vu.begin(), vu.end(), greater<int>() );
~~~

Same sort, different outcome because of different policy.

### Binding extends what policies can do
We can further modify the behavior of policies by sending additional arguments along with the policy as it goes to the algorithm.  This is called binding an argument to a function call. For example, let's find the first value in our vector that is >= 5:

~~~{.cpp}
  //we need these headers
  #include <functional>
  #include <iostream>

  vector<unsigned>::iterator itr = find_if( vu.begin(), 
  vu.end(), 
  bind2nd( greater_equal<int>(), 5 ) );

  //print out the value pointed to by the iterator, 
  //if and only if we found something.  In C++,
  //when a policy never finds anything, 
  //the end of the container is returned by tradition
  if( itr != vu.end() )
  {
    cout << *itr << '\n';
  }
~~~

The find_if algorithm takes each value in the range and evaluates it via the policy.  Here, the policy is greater_equal, which takes two arguments.  The second argument is provided by using the standard-library function bind2nd, which results in the value 5 being the second value passed to the policy's operator().  The new binders in C++11 are much better:

~~~{.cpp}
  //we need these headers
  #include <functional>
  #include <iostream>

  vector<unsigned>::iterator itr = find_if( vu.begin(), 
  vu.end(), 
  //here, _1 is a placeholder for a value that the algorithm must provide
  std::bind( greater_equal<int>(),std::placeholders::_1, 5 ) );

  //print out the value pointed to by the iterator, 
  //if and only if we found something.  In C++,
  //when a policy never finds anything, 
  //the end of the container is returned by tradition
  if( itr != vu.end() )
  {
    cout << *itr << '\n';
  }
~~~


## Summary so far

* Policies change how algorithms behave
* Policies are often templates
* Policies are often function objects
* Policies + binders + algorithms = a reusable code base that can do lots of different (and often quite complicated) things when the right policy is written.
* Policies can often be quite short to implement (see the definition of less above).  This doesn't have to be the case, but it often works out that way in practice.

\section TutAlgo Algorithms in fwdpp

For individual-based simulations, the primary algorithms are __KTfwd::sample\_diploid__ and __KTfwd::update\_mutations__.  All of these functions are in the name space __KTfwd__ and are documented both in the source code, via the doxygen output based on the source code, and finally in the example code that comes with the library.

Some of the code in some of these algorithms is quite complex, largely because generic templates can have hideous syntax.  You should't have to worry about that unless you like seeing how the sausage is made.  Most users of __fwdpp__ will be writing custom policies to stick into these algorithms and will likely be starting from the examples in order to get oriented.  For them, the necessary information is to understand what is required of a policy.  That is the subject of the remainder of this document.

\section TutPolicies Policy requirements in fwdpp

This section discusses the requirements placed on policies in __fwdpp__.  These requirements are essentially standards placed on data types in order to ensure that simulations behave properly. (Note that ""behave properly'' is not the same as "are implemented correctly''!  It is totally possible to have a simulation that compiles with no warnings and runs without crashing but is not the model you had in mind.)  The policy requirements are enforced during compilation, such that a nonconforming policy cannot result in a compiled simulation program.

In the following sections, we will discuss policy requirements, how the built-in policies work, and also create some new policies.  We'll build some policies for a simple quantitative trait with selection simulation, and later on combine them all to see how __KTfwd::sample\_diploid__ would actually be called (for the case of a constant-sized population in an individual-based simulation).  For additional and more complex examples, see the code for the example programs that come with the library.

Caveat emptor:

1. This document has been written ad lib, and there may be errors in the implementation of policies below.  (The policies that are directly from the library have been copy/pasted, and so will be ok.)
2. The code blocks below are really pseudocode.  The include directives are put in there are guides.   Obviously, none of what is below constitutes the complete implementation of a simulation.  For that, see the examples that come with the library.


## Short version for the impatient

In this section, I define the _minimal_ requirements that a policy must conform to.  I also provide pseudocode in the form of lambda expressions in order to illustrate these requirements.  By "requirements'', I mean "things that must be true, otherwise a program using __fwdpp__ will fail to compile.''  Because __fwdpp__ is a template library, policy errors are caught at compile-time.  (Biological errors in your policies, however, will result in run-time issues.  For example, doing your fitness calculations incorrectly, etc.)  When policies require more than the minimal requirements described here, the programmer sends them along via either the usual "bind'' mechanisms or via capature in lambda expressions.

I will describe the minimap policy requirements in terms of their equivalent declarations as objects of type [std::function](http://en.cppreference.com/w/cpp/utility/functional/function), which is a (variadic) C++11 template type that is used like this:

~~~{.cpp}
#include <cmath>
#include <functional>

//A function returning a double and taking two const double & as arguments
std::function< double(const double &, const double &) > mypow = [](const double & a, const double & b) { return std::pow(a,b); };

//A function returning a double and taking three const double & as arguments
std::function< double(const double &, const double &,const double &) > mypow_over_x = [](const double & a, const double & b, const double & c) { return std::pow(a,b)/c; };

//A function returning a double and taking a single const double & as argument.  It is implemented in terms of a wrapper to the previous function.
std::function<double(const double &)> mypow_over_x_wrapper = std::bind(mypow_over_x,3,2,std::placeholders::_1);

int main()
{
  std::cout << mypow(3,2) << '\n';
  for(unsigned i=1;i<11;++i)
    {
      //These two calls will give the same result:
      std::cout << mypow_over_x(3,2,i) << ' ' <<  mypow_over_x_wrapper(i) << '\n';
    }
}
~~~

So, std::function is a concrete type defining the _signature_ of a function (return type and arguments).  Internally, that signature must be met by your policies, and you can "bind" additional arguments to them in order to implement specific models.

For the sake of being explicit, let's assume that we are using the following types in an individual-based simulation (in C++11, "using'' statements replace typedefs.):

~~~{.cpp}
using mtype = KTfwd::mutation;
using mlist = std::list<mtype>;
using gtype = KTfwd::gamete_base<mtype>;
using glist = std::list<gtype>;
~~~

A minimal mutation policy returns an __mtype__ and takes one of the following sets of arguments:

* A non-const reference to a "recycling bin" and a pointer to a mutation list
* A non-const reference to a "recycling bin", a non-const reference to an iterator to a gemete and a pointer to a mutation list.

See KTfwd::infsites for examples of mutation policies.

When a gamete is mutated, we need to decide how to insert it into the __glist__.  This "gamete insertion policy'' takes a const reference to an __gtype__ and a non-const pointer to an __glist__ as arguments, and returns an __glist::iterator__.  Again, under the infinitely-many sites model, a gamete with a new mutation is guaranteed to be unique, so we can simply insert it:

~~~{.cpp}
std::function<glist::iterator(const gtype &, glist *)> gamete_insertion_policy = [](const gtype & g,glist * __gametes) { 
return __gametes->insert(__gametes->end(),g); 
};
~~~

A fitness policy takes two const references to __glist::const\_iterator__ as arguments and returns a double:

~~~{.cpp}
std::function<double(const glist::const_iterator &, const glist::const_iterator &)> fitness_policy = [&](const glist::const_iterator & __gamete1,
const glist::const_iterator & __gamete2) {	 
  //No selection!
  return 1.;
}
~~~

See the header file __fwdpp/fitness\_models.hpp__ for examples of fitness policies that I provide for "standard'' cases.

A recombination policy takes two non-const references to __glist::iterator__, scrambles up those gametes appropriately, and returns an unsigned integer representing the number of crossovers, etc., between the two gametes.  

Typically, recombination would work via a call to __KTfwd::recombine\_gametes__, or the convenience wrapper __KTfwd::genetics101__.  Here is an example of the latter, taken from the example file diploid_ind.cc:

~~~{.cpp}
std::bind(KTfwd::genetics101(),
	//These two placeholders are for  iterators to the two parental gametes
	std::placeholders::_1,std::placeholders::_2,
	//This placeholder is for a lookup table
	std::placeholders::_3,
	//This placeholder is for a "recycling bin"
	std::placeholders::_4,
	//These two containers should be allocated in main (or via the appropriate sugar type, _e.g._ KTfwd::singlepop_t)
	//They often need reference wrappers to make the templates compile.
	//They are used to hold the intermediate results of crossover events
			std::ref(neutral),std::ref(selected),
				&pop.gametes,
					littler,
						r.get(),
							recmap)
~~~

See @ref md_md_algo for more details about what the lookup table and the "neutral" and "selected" containers do.

For individual-based simulations with migration, returns an unsigned integer and takes on as an argument:

~~~{.cpp}
/*
Simple migration policy for two-deme simulation
*/
size_t migpop(const size_t & source_pop, gsl_rng * r, const double & mig_prob)
{
  if( gsl_rng_uniform(r) < mig_prob )
    {
      return ! source_pop;
    }
  return source_pop;
}
//Assume r and mig_prob are defined somewhere...
std::function< unsigned(const unsigned &) > migpol = std::bind(migpop,std::placeholders::_1,r,mig_prob);
~~~

## Technicalities related to policy "types"

C++11 programmers will note that judicious use of "auto" will make all of the above code more streamlined.  Further, policies could be implemented in many cases as one or more of the following:

* functions
* function objects
* lambda expressions
* expressions synthesized from any of the above using std::bind

Further, the functions and function objects could themselves be implemented in terms of templates, meaning there are lots of ways to do things.

The use of std::function in the preceding section makes the return type and argument requirements explicit.  Further, it is often convenient to create policies by mixing std::bind and lambda expressions.  Let's revisit our previous C++ example:

~~~{.cpp} 
#include <cmath>
#include <functional>
#include <iostream>

auto mypow_over_x= [](const double & a, const double & b, const double & c) { return std::pow(a,b)/c; };
auto mypow_over_x_wrapper1 = [&](const double & a, const double & b, const double & c) { return mypow_over_x(a,b,c); };
//This is the same as "wrapper1"
auto mypow_over_x_wrapper2 = [&](const double & a, const double & b, const double & c) { return mypow_over_x(a,b,c); };

//These two are the same as wrappers 1 and 2...
std::function<double(const double &,const double &, const double &)> mypow_over_x_wrapper3 = [&](const double & a, const double & b, const double & c) { return mypow_over_x(a,b,c); };
std::function<double(const double &,const double &, const double &)> mypow_over_x_wrapper4 = [&](const double & a, const double & b, const double & c) { return mypow_over_x(a,b,c); };

int main()
{
  //This will evaluate to 0!!!!
  std::cout << (typeid(mypow_over_x_wrapper1).name() == typeid(mypow_over_x_wrapper2).name()) << '\n';
  //This will evaluate to 1!!!!
  std::cout << (typeid(mypow_over_x_wrapper3).name() == typeid(mypow_over_x_wrapper4).name()) << '\n';
}
~~~

The lesson: lambda expression __always__ have different signatures.  This matters for the metapopulation and multi-locus simulations where you must provide _vectors_ of policies, _e.g._:

~~~{.cpp}
//The only way to have a vector of policies is to force them to have the same signature ("type"):
std::vector< std::function< return_value( arg1_type, arg2_type, etc. )> > policies;
~~~

\subsection TutMut Mutation policies
This is the mutation base class  provided by __fwdpp__

~~~{.cpp}
  struct KTfwd::mutation_base
  {
    /// Mutation position
    mutable double pos;
    /// Count of mutation in the population
    unsigned n;
    /// Is the mutation neutral or not?
    bool neutral;
    /// Used internally (don't worry about it for now...)
    bool checked;
    mutation_base(const double & position, 
    const unsigned & count, const bool & isneutral = true)
      : pos(position),n(count),neutral(isneutral),checked(false)
    {	
    }
    virtual ~mutation_base(){}
};
~~~

The above code defines a mutation as something with a position (stored as a double), a count (unsigned integer), a boolean declaring the mutation to be neutral or not, and another boolean called "checked'' which is very important but should only be directly manipulated by internal library functions (unless you really geek out and see what the internals are doing.  In that case--go nuts.)

The mutation base class is not sufficient for any interesting sorts of simulations.  Rather, one must derive a class from it with more data types.  The library provides a class called mutation, which is probably the standard type of mutation that a population geneticist would think of (this class is also in the library's namespace KTfwd):

~~~{.cpp}
  struct mutation : public mutation_base
  //!The simplest mutation type, adding just a selection 
  //coefficient and dominance to the interface
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
      return( fabs(this->pos-rhs.pos) <= std::numeric_limits<double>::epsilon() &&
	      this->s == rhs.s );
    }
};
~~~

What does a mutation policy (model) need to do?  __The answer is that a single call to the mutation model function (or function object) must return a single instance of the simulation's mutation type with a count of 1.__

### Example: the infinitely-many sites model of mutation

This mutation model states that a new mutation occurs at a site not currently segregating in the population.  This statement implies the following:

1. We need a method to rapidly choose mutation positions that don't currently exist in the (meta-)population.
2. Each gamete containing a new mutation is by definition a new gamete in the (meta-)population.  If we did \#1 correctly, then the newly-mutated gamete differs from all others in the population by at least 1 new mutation.


We will now implement this mutation model for the mutation type "mutation'' defined above.  In order to add some complexity to our mutation model, we will make the additional modeling assumptions:

1. Mutation positions are continuous on the interval \f$[0,1)\f$.
2. Neutral mutations arise at rate \f$\mu\f$ per gamete per generation
3. Selected mutations arise at rate \f$\mu_s\f$ per gamete per generation.
4. The selection coefficient for a newly-arising mutation is exponentially-distributed with mean \f$s_m\f$.  Further, half the time, selected mutations are deleterious (\f$s < 0\f$).  Otherwise, they are beneficial (\f$s > 0\f$).
5. Dominance will be uniform from 0 to 2.  (We'll be scaling fitness as \f$1, 1+hs, 1+2s\f$ for genotypes AA, Aa, and aa, respectively.)


From a programming point of view, we need a means to lookup all mutation positions currently segregating in the population.  __fwdpp__ provides support for lookup tables that conform to the behavior of the type __std::map__.  While one could use a type like 

~~~{.cpp}
  std::map<double,bool>
~~~

it is more efficient to use a hash table like 

~~~{.cpp}
  #include <unordered_set> //need this header
  typedef std::unordered_set<double,std::hash<double>,KTfwd::equal_eps > lookup_table_type;
~~~

The above code creates a new data type called __lookup\_table\_type__ that hashes doubles with the data type __KTfwd::equal\_eps__ as its comparison operator.  That comparison operation is provided by the library and looks like this:

~~~{.cpp}
  struct equal_eps
  {
    typedef bool result_type;
    template<typename T>
    inline bool operator()(const T & lhs, const T & rhs) const
    {
      return( std::max(lhs,rhs)-std::min(lhs,rhs) <= std::numeric_limits<T>::epsilon() );
    }
  };
~~~

Note that you could provide your own equality comparison policy for the hashing table.  This one would be excellent, and should be included in the library in future versions as it may be the most robust:

~~~{.cpp}
  struct equality_comparison_strict
  {
    typedef bool result_type;
    template<typename T>
    inline bool operator()(const T & lhs, const T & rhs) const
    {
      return( !(lhs > rhs) && !(lhs < rhs) );
    }
  };
~~~

We can now completely define our mutation model as a function (we could do it as a function object, too).  In this example, We assume that we are using the boost list type and boost's memory pool allocator:

~~~{.cpp}
  typedef KTfwd::mutation mtype;
  typedef boost::pool_allocator<mtype> mut_allocator;
  typedef boost::container::list<mtype,mut_allocator > mlist;
  mtype mutmodel( gsl_rng * r,
                  const double & mu_neutral,
                  const double & mu_selected,
                  const double & mean_s,
                  lookup_table_type * lookup )
    {
      //get new mutation position
      double pos = gsl_rng_uniform(r);
      //this is very rapid lookup...
      while( lookup->find(pos) != lookup->end() )
      {
        pos = gsl_rng_uniform(r);
      }
      //ok, we have new position, so put it in lookup table
      lookup->insert(pos);

      //law of TTL prob
      bool neutral = (gsl_rng_uniform(r) < (mu_neutral)/(mu_neutral+mu_selected)) ? true : false;

      //return neutral mutation
      if ( neutral ) { return mtype(pos,0,1,0); }

      //get selection coefficient
      double s = gsl_ran_exponential(r,mean_s);
      if( gsl_ran_uniform(r) <= 0.5 ) { s = -1.*s; }

      //the gsl_ran_flat call generates the dominance
      return mtype(pos,s,1,gsl_ran_flat(r,0.,2.));
    }
~~~

That is is--the mutation model is complete.  We still need to deal with how mutations are entered into data structures representing the population, but we'll treat that later.

The mutation policy is passed to any of the various __KTfwd::sample\_diploid__ functions in the library like this:

~~~{.cpp}
  std::bind(mutmodel,r, mu_neutral, mu_selected, mean_s, &lookup);
~~~

Or, using C++11 lambda expressions:

~~~{.cpp}
  [&](){ return mutmodel(r,mu_neutral, mu_selected,mean_s,&lookup); }
  ~~~

Note that both of the above declarations are the first of the four types of mutation moel alluded to above--they take no further arguments from the library's internals, and their signature is thus

~~~{.cpp}
std::function<mtype(void)>
~~~

as far as the library is concerned.

Note that several of the data types passed to the model are non-const pointers.  Therefore, it is very likely that the data pointed to will be modified my the mutation model!

### A model for quantitative trait simulations
Let's define a model where a mutation affecting fitness does so via its effect size, \f$e\f$, which is Gaussian-distributed with mean zero and standard deviation \f$\sigma_e\f$.

We need a mutation class:

~~~{.cpp}
  struct mut_e : public KTfwd::mutation_base
  {
    double e;
    mut_e( const double & __pos,
           const unsigned & __n,
           const bool & __neut,
           const double & __e ) : KTfwd::mutation_base(__pos,__n,__neut),e(__e)
           {
           }
  };

 typedef mut_e mtype;
~~~

OK, our mutation model is going to be the following.  It is infinitely-many sites with both neutral and non-neutral mutations:

~~~{.cpp}
   typedef KTfwd::mutation mtype;
   typedef boost::pool_allocator<mtype> mut_allocator;
   typedef boost::container::list<mtype,mut_allocator > mlist;
   mtype mutmodel_Qtrait( gsl_rng * r, mlist * mutations,
                          const double & mu_neutral,
                          const double & mu_selected,
                          const double & sigma_e,
                          lookup_table_type * lookup )
    {
      //get new mutation position
      double pos = gsl_rng_uniform(r);
      //this is very rapid lookup...
      while( lookup->find(pos) != lookup->end() )
      {
        pos = gsl_rng_uniform(r);
      }
      //ok, we have new position, so put it in lookup table
      lookup->insert(pos);

      //law of TTL prob
      bool neutral = (gsl_rng_uniform(r) < (mu_neutral)/(mu_neutral+mu_selected)) ? true : false;

      //return neutral mutation
      if ( neutral ) { return mtype(pos,1,true,0); }

      //the gsl_ran_gaussian call determines the effect size
      return mtype(pos,1,gsl_ran_gaussian(r,sigma_e),false);
    }
~~~


\subsection TutRec Recombination

### How recombination is modeled in __fwdpp__

Currently, recombination is modeled as follows.  In a diploid, there number of crossovers between gametes is Poisson distributed with mean \f$r\f$.  There is no notion of interference in establishing crossover positions.  (However, that can be done with a clever policy.)

### Recombination map functions
In __fwdpp__, a recombination map is a function or function object that returns a double and takes no additional arguments from the algorithm.  The return value is the position of the crossing over event.  The simplest recombination map is uniform.  Here is how to implement such a map on the interval \f$[0,1)\f$:

~~~{.cpp}
  #include <functional>
  #include <gsl/gls_rng.h>
  //gsl_rng * r assumed to be initialized already...
  std::function< double(void) > recmap = std::bind( gsl_rng_uniform, r);
~~~

Let us write a recombination map function that models a strong hotspot of crossing over.  The positions will still be on the interval \f$[0,1)\f$.  A fraction \f$p\f$ of the recombination events will come from a uniform distribution and \f$1-p\f$ will come from a beta distribution with parameters \f$a\f$ and \f$b\f$.  The genetic map looks like this:

~~~{.cpp}
  double mixture_map(gsl_rng * r, 
                     const double & p,
                     const double & a,
                     const double & b)
      {
        return (gsl_rng_uniform(r) < p) ? gsl_rng_uniform(r) : gsl_ran_beta(r,a,b);
      }
  /*
     This is a hot hotspot.  say hist( c(runif(1e3),rbeta(9e3,100,100) ) ) in R 
     to see density of crossover positions
  */
  std::function< double(void) > recmap = std::bind( mixture_map,r,0.1,100,100);
~~~

Either of the above code blocks results in a variable called recmap which is a function object representing a function call that takes no additional arguments and returns a double.  The variable recmap can be passed to the algorithm as the recombination (sometimes called genetic in the library documentation) map policy.

Note that the above policies were implemented by _synthesiszing_ a new function object type from a stdd::bind operation via the std::function template class.  You may synthesize all of your policies into variables this way, but it is not required.  However, the next subsection will reveal a case where it is required.

### Recombination model policy functions

Individual-based simulations require a recombination model policy.  This policy is a function/function object that takes two non-const references to iterators to gametes as arguments and returns an unsigned integer representing the number of crossovers between the two gametes.  (Note that the return value will never been seen by a library user.  It basically exists to help debug things deeper in the library if and when it comes to that.)  

The library provides the policy __KTfwd::genetics101__ which implements the model of crossing over described above.

The policy looks like this:


~~~{.cpp}
	  struct genetics101
  /*! Genetics 101: simple model of recombination.  r is the probability that the two gametes recombine
   */
  {
    using result_type = unsigned;
    template<typename gamete_iterator_type,
	     typename gamete_list_type_allocator,
	     typename glookup_t,
	     template<typename,typename> class gamete_list_type,
	     typename rec_pos_generator>
    unsigned operator()( gamete_iterator_type & g1,
			 gamete_iterator_type & g2,
			 glookup_t & gamete_lookup,
			 typename gamete_iterator_type::value_type::mutation_container & neutral,
			 typename gamete_iterator_type::value_type::mutation_container & selected,
			 gamete_list_type< typename gamete_iterator_type::value_type, gamete_list_type_allocator > * gametes,
			 const double & littler,
			 gsl_rng * r,
			 const rec_pos_generator & rp) const
    {
      unsigned NREC = 0;
      if( g1 != g2 )
	//then a non-parental type is inherited from p1 and p1 has two different gametes
	{
	  NREC += recombine_gametes(r,littler,gametes,g1,g2,gamete_lookup,neutral,selected,rp);
	}
      return NREC;
    }
  };
~~~

The main thing a library user needs to focus on is the argument list for __operator()__.  Specifically, it requires a variable of type __rec\_pos\_generator__ which is stated in the documentation to be a recombination map policy. Thus, a recombination model is a policy that requires another policy.  Further, __a recombination policy is passed non-const references to iterators to two gametes (g1 and g2 in the code above). Those iterators point to the parental gametes, and (as of fwdpp 0.3.3), they get "swapped' half to time to represent Mendelian segregation.  Ultimately, g1 represents the chromosome that will be passed on to the offspring, and the data that it points to will be modified by the library's internal functions for recombination.  See @ref md_md_algo for more details on how this works.  (The details of crossing over changed radically in version 0.3.3, resulting in much faster simulations.)

We pass this recombination model in an individual-based simulation to KTfwd::sample_diploid like this:

~~~{.cpp}
  using std::placeholders; //_1,_2, etc.
  std::bind(KTfwd::genetics101(),  //the rec. model
  _1,_2,                  //placeholder for iterators to gametes
  _3, //placeholder for a lookup table
  std::ref(neutral),std::ref(selected), //containers for intermediate results
             &gametes,               //pointer to gamete list
             littler,                //Avg. # of crossovers b/w two gametes per region per generation
             r,                      //a gsl_rng *
             recmap)                 //A genetic map policy like the one we made above
~~~

Note: if you want to write a new recombination policy, you probably want to proceed by modifying how it interacts with the the recombination map policy.  For example, if a recombination at position \f$x\f$ means that the next position must be \f$\geq 1.5x\f$ (in some model of interference).  Doing so requires making a custom version of the __KTfwd::recombine\_gametes__ function.  If you read the code for that function, you will see where the recombination map policy is called.  If you try to modify the code below that, then good luck to you.  It isn't super-complicated, but tread with caution.

\subsection TutMig Migration
Migration policies are only used in individual-based simulations.

For individual-based simulations involving a metapopulations, parent 1 comes from population \f$i\f$ and may or not be a migrant.  Parent two comes from population \f$j\f$ and \f$j = i\f$ in the case of no migration, otherwise \f$j \neq i\f$.  Migration policies may be the trickiest to write effectively because spatial models of migration can be complicated.  However, a migration policies requirements are simple.  A migration policy is a function or function object taking an argument if type size\_t and returning a value of type size\_t.  The argument is the index of population \f$i\f$, and the return value is the index of population \f$j\f$.

For example, let's assume two demes with migration rate \f$m\f$.  Here, \f$m\f$ is the probability that a parent is a migrant. This migration rate is equal between the two demes.  Because we are in a C-like language, the values allowed for the __size\_t__ are \f$0 \leq i \leq 1\f$.  The migration policy is thus defined as follows:

~~~{.cpp}
  size_t migpop(const size_t & source_pop, gsl_rng * r, const double & mig_prob)
  {
    //if parent is a migrant
    if( gsl_rng_uniform(r) < mig_prob )
    {
      //return other population
      return ! source_pop;
    }
    //else, not a migrant
    return source_pop;
  }
~~~

And we pass it to __KTfwd::ample\_diploid__ like this:

~~~{.cpp}
  std::bind(migpop, //the policy
  std::placeholders::_1,     //placeholder for population index i
  r,     //gsl_rng *
  m)     //migration rate
~~~

\subsection TutFitness Fitness
The ability to define custom fitness policies is perhaps the most useful feature of __fwdpp__.  Broadly-speaking, there are two typical types of fitness models used in population genetics.  The first are what I call site-based models, such as the model of multipicative fitness across sites.  In this standard model, each non-neutral site is effectively its own gene. (A trans-heterozygotoe for two recessive mutations has wild-type fitness under the multiplicative assumption. That satisfies Benzer's definition of complementation, which is the operational definition of a gene.)  The second class are models are haplotype- or region- based, in that fitness depends on the effect sizes of the maternal and paternal haplotypes that a diploid inherited.  The library supports both types.  Haplotype-based fitness models are often efficient to compute, but site-based models can be done badly.  The library provides additional assistance for site-based models.

__A fitness policy is a function or function object taking two iterators pointing to gametes are arguments are returning a double.  The pointers are the diploid's haplotypes, and the return value is the fitness.__
### Site-dependent models of fitness

The library provides a fitness policy called __KTfwd::site\_dependent\_fitness__.  This function object requires two additional policies defining what to do with homozygous sites and heterozygous sites.  These "homozygote'' and "heterozygote'' policies may be trivially defined as lambda expressions:

~~~{.cpp}
  struct multiplicative_diploid
  {
    typedef double result_type;
    template< typename iterator_type>
    inline double operator()(const iterator_type & g1, const iterator_type & g2,
                             const double scaling = 1.) const
    {
      using __mtype = typename iterator_type::value_type::mutation_list_type_iterator;
      return site_dependent_fitness()(g1,g2,
      //Homozygote policy
				      [&](double & fitness, const __mtype  & mut)
				      {
					fitness *= (1. + scaling*mut->s);
				      },
                                      //Heterozygote policy
				      [](double & fitness,const __mtype & mut)
				      {
					fitness *= (1. + mut->h*mut->s);
				      },
                                      1.);
    }
  };
~~~

That last variable, scaling, means that fitness is the product of \f$1, 1+sh, 1+s \times \mathrm{scaling}\f$ over sites.  This allows you to recreate results from the different parts of the literature that use \f$scaling = 1\f$ and 2.  Many classic results are based on a scaling of 2.

To use the multiplicative fitness policy in your simulations, this goes to __KTfwd::sample\_diploid__:

~~~{.cpp}
  std::bind(multiplicative_diploid(),std::placeholders::_1,std::placeholders::_2,2.)
~~~

If you wish to create your own site-dependent fitness policies, the recipe is:

1. Create your own versions of the policies determining what happens in diploids homozygous vs. heterozygous for a mutation.  __These policies are passed the current fitness from the algorithm and an iterator to a mutation object.__
2. Write a wrapper function like multiplicative\_diploid that passes these policies to __KTfwd::site_dependent_fitness__,  _which itself is a policy requiring that iterators to two gametes, the homozygote/heterozygote policies, and a starting value for fitness be passed to it._  In the example of the built-in multiplicative fitness policy, that value of \f$1\f$ that is passed as the last argument to __KTfwd::site\_dependent\_fitness__ is the initial value of fitness for a diploid.  In other words, \f$w = 1\f$ initially, and \f$w\f$ is then modified by the policies for what to do with homo- and hetero- zygous genotypes.
3. This wrapper function is your new fitness policy.

Note: the implementation  __KTfwd::site\_dependent\_fitness__ is a lot of iterator/pointer arithmetic.

### Haplotype based fitness policies
Here is one that assume a user-defined mutation type with effect size \f$e\f$ associated with it.  The effect of a haplotype is additive, the genetic model is recessive, and the phenotype is the genetic effect + a Gaussian random variable with standard deviation sigma.  Finally, fitnesses are under Gaussian stabilizing selection with a standard deviation of 1 and mean 0.

~~~{.cpp}
struct hapfitness
{
  typedef result_type double;
  template<typename gam_itr
  double operator()(const gam_itr & g1,
                    const gam_itr & g2,
                    gsl_rng * r,
                    const double & sigmaE) const
   {
     double sum1=0.,sum2=0.;
     typedef typename gam_itr::value_type::mutation_container::const_iterator mci;
     for( mci mitr = g1.smutations.begin() ; mitr != g1.smutations.end() ; ++mitr )
     {
       //mitr is an iterator to an iterator!
       sum1 += (*mitr)->e;
     }
     for( mci mitr = g2.smutations.begin() ; mitr != g2.smutations.end() ; ++mitr )
     {
       //mitr is an iterator to an iterator!
       sum2 += (*mitr)->e;
     }

     //make sum1 be the value closest to 0
     if( fabs(sum1) > fabs(sum2) ) { std::swap(sum1,sum2); }

     //add noise to fitness
     //Using sum1 as the genetic part makes the model recessive
     double pheno = sum1 + gsl_ran_gaussian(r,sigmaE);
     
     /*
       Return fitnees under Gaussian stabilizing model.
       This is the only part of the Gaussian pdf that
       matters.  The rest is a constant
       and so won't affect sampling prop to fitness.
      */
     return std::exp( -std::pow(pheno,2) /2. );
   }
};
~~~

To use the above policy:

~~~{.cpp}
	std::bind( hapfitness(),
        std::placeholders::_1,std::placeholders::_2,
	r,sigmaE );
~~~

Please note that a long-running annoyance with open-source C++ compilers (GCC!) is whether or not exp, pow, log, etc., are in namespace std or in the global namespace.  This can vary from version to version and across operating systems.  Sometimes, you need to say __std::exp__ when on another system that fails and you need __::exp__.

\subsection TutUpdateRemove Updating and removal policies

During the course of a simulation, new mutation and gamete types come and go.  New types must be entered in to their approproate containers.  The relevant policies are basically searches followed by either a member variable update or an insertion.  For example, consider our mutation model policy in section \ref{infsites}.  The mutation returned from that function is not currently found in our mutation list (because it has a unique position).  Therefore, the relevant policy is just to insert the new mutation at the end of the doubly-linked list of mutations.  Likewise, a gamete with a new mutation from that model cannot be identical to any currently-segregating gamete.  Therefore, a good policy is just to insert it at the end of the doubly-linked list of gametes.  However, a recombination event could make either a new gamete or a pre-existing gamete.  Thus, a policy to insert a recombinant gamete should do the following:


1. Figure out of the gamete is new or not.
2. If it is new, add it to the end of the gamete container.
3. It it is not new, do something intelligent, such as return the iterator to the pre-existing version of that gamete in the container.

These types of policies are simple.  See the header files __fwdpp/insertion\_policies.hpp__ and __fwdpp/fwd\_functional.hpp__. 

\section TutTieItup Putting it all togeter (kinda)

Ok, once we have defined our mutation types, our containers, and our policies, a single generation of a constant-sized population in an individual-based simulation is evolved a follows:

~~~{.cpp}
  using std::placeholders; //_1,_2, etc.
  double wbar = KTfwd::sample_diploid(r,
                                      &gametes,  //non-const pointer to gametes
                                      &diploids, //non-const pointer to diploids
                                      &mutations, //non-const pointer to mutations
                                      N,     //current pop size, remains constant
                                      mu,    //mutation rate per gamete
                                      /*
                                      The mutation model (defined above) will pass each gamete
      					 to be mutated to the mutation model function.  Again, _1
      					 is used as a placeholder for that gamete.
      				       */
      				       std::bind(mutmodel_Qtrait,r,_1,mu_neutral,mu_selected,sigma_e,&lookup),
				       //The recombination policy includes the recombination map policy
      				       std::bind(KTfwd::genetics101(),_1,_2,_3,_4,
						   &gametes,
      						   littler,
      						   r,
      						   recmap),
				       /*
					 Policy telling KTfwd::mutate how to add mutated gametes into the gamete pool.
					 If mutation results in a new gamete, add that gamete to the 
					 end of gametes. This is always the case under infinitely-many sites,
					 but for other mutation models, mutation may result in a new
					 copy identical to an existing gamete.  If so,
					 that gamete's frequency increases by 1.
				       */
      				       std::bind(KTfwd::insert_at_end<gtype,glist>,_1,_2),
      				       std::bind(  std::bind( hapfitness(),
                                                   _1,_2,
                                                   r,sigmaE ),
      				       /*
                                         Only remove lost mutations
      				       */
      				       std::bind(KTfwd::mutation_remover(),_1,0));
          //Clean up the mutations list.  This also resets "checked'' in each mutation to zero,
          //which is that "internal detail'' referred to above
      	  KTfwd::update_mutations(&mutations,&lookup);
~~~
