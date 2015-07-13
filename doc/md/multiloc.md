# Tutorial 2: Implementing multilocus simulations

In order to simulate discontiguous genomic segments, one has two options within __fwdpp__:

1. Write mutation, recombination, and fitness policies that "do the right thing" for your model.  For example, a mutation policy would need to know about the mutation rate at each locus, and appropriately assign mutations with the correct positions, fitness effects, etc.
2. Write separate mutation and recombination policies for each locus, and a fitness policy that calculates the fitness of a diploid over all loci.

This document covers the latter method, which I call the "multilocus" part of __fwdpp__.    I won't give any examples of the former method, as I'm opposed to the idea of having to develop, debug, and mainting large complex policies.  But if you want to do things that way, read the [tutorial on policies](@ref md_md_policies), because what you are looking for is all in there.

# Simple policies using the multi-locus machinery

The main conceptual difference between this part of the library and the examples shown in the [tutorial on policies](@ref md_md_policies) is the following:

* Instead of a single mutation model policy, you implement one mutation model per "locus".  These policies are stored in a vector and passed to KTfwd::sample_diploid.
* Similarly, you implement a recombination policy per locus, and pass a vector of those policies along to KTfwd::sample_diploid.
* Instead of a single list of gametes, you have a vector of lists of gametes, where each list represents the current gametes at a particular locus.
* A diploid is now represented as a vector of pairs of iterators derived from the vector of lists of gametes.
* A fitness policy calculates individual fitnesses from an iterator pointing to that vector of pairs of iterators.

At this point, it may be most useful to look at a concrete example.  The program diploid_ind_2locus.cc is distributed with the library source code, and we'll break down its essential parts in the next few sections.

### A multilocus mutation model

For this example, we simulate only neutral mutations, and the model has two loci.  Our function to return a new mutation will do the following:

1. Take the start position of a locus as an argument.
2. Return a new mutations whose position is uniformly-distributed on in interval \f$(\mathrm{start},\mathrm{start}+1]\f$

This is our mutation class:

~~~{.cpp}
struct mutation_with_age : public KTfwd::mutation_base
{
  unsigned g;
  double s,h;
  /*
    The constructor initializes all class data, including that of the base class via a constructor
    call to the base class.
  */
  mutation_with_age(const unsigned & __o,const double & position, const unsigned & count, const bool & isneutral = true)
    : KTfwd::mutation_base(position,count,isneutral),g(__o),s(0.),h(0.)
  {	
  }
};
~~~

It contains info for selection coefficients, etc., but we won't be using any of that.  (It is only there b/c I use the same mutation object for all of the example programs...)

Given the requirements outlined above, a function to return a mutation at a specific locus is:

~~~{.cpp}
//"beg" is the start position of this locus
mutation_with_age neutral_mutations_inf_sites(gsl_rng * r,const unsigned * generation,mlist * mutations,
					      lookup_table_type * lookup, const double & beg)
{
  //Generate new mutation position on the interval [0,1)
  double pos = gsl_ran_flat(r,beg,beg+1.);
  while( lookup->find(pos) != lookup->end() ) //make sure it doesn't exist in the population
    { 
      pos = gsl_ran_flat(r,beg,beg+1.);
    }
  //update the lookup table
  lookup->insert(pos);

  //In absence of DEBUG, make sure lookup table is working
  assert(std::find_if(mutations->begin(),mutations->end(),std::bind(KTfwd::mutation_at_pos(),std::placeholders::_1,pos)) == mutations->end());

  //return constructor call to mutation type
  return mutation_with_age(*generation,pos,1,true);
}
~~~

The details of the lookup table are covered in the main [tutorial](@ref md_md_policies) on policies.  It serves to efficiently ensure that we are sampling new mutation positions from an infinitely-many sites model.

We may now create mutation model policies by synthesizing a function call that returns a mutation_with_age using [std::bind](http://en.cppreference.com/w/cpp/utility/functional/bind):

~~~{.cpp}
auto mmodel0 = std::bind(neutral_mutations_inf_sites,r,&generation,std::placeholders::_1,&lookup,0.);
auto mmodel1 = std::bind(neutral_mutations_inf_sites,r,&generation,std::placeholders::_1,&lookup,1.);
~~~

The placeholder is for a pointer to a doubly-linked list of mutations (see argument 3 in the definition of neutral_mutations_inf_sites above).  Each of these is identical to what we do in a single-locus simulation.  The only "trick" here is that we pass a 0 to mmodel0, and a 1 to mmodel1 to represent the starting position for each locus, thus guaranteeing that neutral_mutations_inf_sites returns a mutation with positions \f$(0,1]\f$ or \f$(1,2]\f$.

We need to store these policies in a vector.  The easiest way to do that is to use the C++11 features [decltype](http://en.cppreference.com/w/cpp/language/decltype) and [list initialization](http://en.cppreference.com/w/cpp/language/list_initialization):

~~~{.cpp}
std::vector<decltype(mmodel0)> mmodels { mmodel0, mmodel1 };
~~~

### Caveat: vectors of policies containing values captured in lambda expressions.

The above examples all consider a fixed number of loci.  However, if the number of loci is to be determined at run-time, then your mutation models, etc., will need to be created dynamically.  When using _lambda expressions_, be sure to capture _by value_ and not _by reference_.  For example, this loop results in a vector of mutation policies where mutation positions at the \f$i^{th}\f$ locus are continuous on the interval \f$[i,i+1)\f$:

~~~{.cpp}
std::vector< std::function<mtype(multiloc_t::mlist_t *)> > mmodels;
for(unsigned i = 0 ; i < nloci ; ++i)
	{
	//Establish the range of positions for this locus
	  double a=i,b=i+1;
	  mmodels.push_back(std::bind(KTfwd::infsites(),r.get(),&pop.mut_lookup,&generation,
	//Capture a,b by VALUE!  Otherwise, they go out of scope and bad things can happen!
		mu[i],0.,[&r,a,b](){ return gsl_ran_flat(r.get(),a,b);},[](){return 0.;},[](){return 0.;}));
	}
~~~

Keep this in mind for all containers of policies (in addition to this  mutation example).

### Digression: type signatures for for policies

The multilocus API requires that the user pass a vector of policies.  A vector's interface further requires that all elements contained by a vector are of the same type.  In the previous section, we used C++11's [auto](http://en.cppreference.com/w/cpp/language/auto) keyword to force the compiler to figure out the type of mmodel0 and mmodel1.  It just so happens that they are of the same type, which I think should be obvious if we consider that both are calls to std::bind with the same number of arguments, all of which are the same type.

Some users may try to implement the mutation models using C++11 [lambda expressions](http://en.cppreference.com/w/cpp/language/lambda):

~~~{.cpp}
auto mmodel0 = [&]( mlist * m ) { return neutral_mutations_inf_sites(r,&generation,m,&lookup,0.); };
auto mmodel1 = [&]( mlist * m ) { return neutral_mutations_inf_sites(r,&generation,m,&lookup,1.); };
std::vector< decltype(mmodel0) > mmodels { mmodel0, mmodel1 };
~~~

However, the above approach will fail because C++11 lambda expressions always have different types, even if their signatures are the same, as in the case above (both lambdas take the same number of arguments, all arguments are the same type, and the return value is the same).  In order to use lambda expressions for our mutation models, our vector of policies will have to specify a particular function signature for its object type using [std::function](http://en.cppreference.com/w/cpp/utility/functional/function):

~~~{.cpp}
std::vector< std::function<mutation_with_age(mlist *)> > mmodels {mmodel0,mmodel1};
~~~

Personally, I prefer the std::bind/decltype idiom here over the lambda/std::function approach, because it means less work on my part.  However, the latter method can win you the greatest number of geek points because it allows you to do everything in one fell swoop and write code that is arguably harder to read:

~~~{.cpp}
std::vector< std::function<mutation_with_age(mlist *)> > mmodels {
	[&]( mlist * m ) { return neutral_mutations_inf_sites(r,&generation,m,&lookup,0.); },
	[&]( mlist * m ) { return neutral_mutations_inf_sites(r,&generation,m,&lookup,1.); }
	};
~~~

In summary, we have to pay some attention to the type signatures of our policies, and lambda expressions require some special care.  We don't have to be aware of this stuff in the single-locus API, because we can rely on the compiler to just work it out for itself during template insantiation.

### Separate within-locus recombination policies

Our with-locus recombination models will be very simple, and will model recombination as a uniform process.  The genetic maps can therefore be defined as:

~~~{.cpp}
std::function<double(void)> recmap = std::bind(gsl_rng_uniform,r),
	recmap2 = std::bind(gsl_ran_flat,r,1.,2.);
~~~

We use the above genetic maps to synthesize recombination policies:

~~~{.cpp}
auto recpol0 = std::bind(KTfwd::genetics101(),std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,&gametes,littler,r,recmap);
auto recpol1 = std::bind(KTfwd::genetics101(),std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,&gametes,littler,r,recmap2);
std::vector< decltype(recpol0) > recpols{ recpol0 , recpol1 };
~~~

Why did I declare the recmaps with std::function instead of auto?  The reason is that the type deduction is more complex here, and the compiler will deduce bizarre signatures for recmap and recmap2 if left to its own devices.  I am still exploring if this can be addressed in the future to allow automatic type deduction via auto/decltype.

### Recombination between loci

To model recombination between loci, you need the following two ingredients:

1. An array of doubles of length \f$L-1\f$, where \f$L\f$ is the number of loci in the simulation.
2. A policy specifying how to apply those values.  For example, you may choose to model crossing over between loci as a binomial process or a Poisson process.  This policy must return an unsinged integer, representing the number of crossovers between loci \f$i\f$ and \f$i-1\f$ (\f$1 \leq i \leq L-1\f$).  What really matters is that the return value from this policy is odd or even.

In our simulation, there are only two loci, so our array may be defined as:

~~~{.cpp}
double rbw = 0. //no crossing over between loci
~~~

And we model crossing over between loci as a Poisson process using a lambda expression:

~~~{.cpp}
[](gsl_rng * __r, const double __d){ return gsl_ran_poisson(__r,__d); }
~~~

Obviously, for this specific example, having no crossover between loci (rbw = 0) make this policy irrelevant, but it does matter in general.

#### Poisson or not?

Biologically, the interpretation of the values in the array of doubles matters.  For example, let's assume that we wanted to model two unlinked loci.  Our genetic classes tell us that this corresponds to:

~~~{.cpp}
double rbw = 0.5; //50 centiMorgans
~~~

However, that value of 0.5 correponds to the probability of success in a single Bernoulli trial, because the definition of the \f$r\f$ term from genetics is the probability that markers separated by that distance are observed to have recombined in the progeny.  If we were to pass 0.5 and a Poisson model like the one above, there would only be a recombinant about 30% of the time, which you can verify using R (noting that an odd number of crossovers corresponds to the terminal ends of the region being recombined in the offspring):

~~~
> x=rpois(1e6,0.5)
> length(which (x%%2==1))/length(x)
[1] 0.315705
~~~

Of course, for small \f$r\f$, the Poisson and the Binomial approach would give very similar results.  But, for larger \f$r\f$, be sure to model what you mean!

### A (trivial) mutilocus fitness model

In this example program, there is no selection, so our fitness model will return 1 for the fitness of any diploid.  Just like the single-locus API, multilocus fitness policies take a diploid as an argument and return a double.  The difference here is that a diploid is now a vector of pairs of iterators to gametes:

~~~{.cpp}
struct no_selection_multi
{
  typedef double result_type;
  //The template type is an iterator derived from the vector containing the diploids,
  //and therefore points to a std::vector< std::pair<gamete_itr_t,gamete_itr_r> >,
  //where the gamete_itr_t are iterators pointing to gametes
  template< typename dipoid_vec_itr_t >
  inline double operator()( const diploid_vec_itr_t & diploid ) const
  {
    return 1.;
  }
};
~~~

### Calling sample_diploid

The above definitions are used to evolve a population via a call to KTfwd::sample_diploid:

~~~{.cpp}
KTfwd::sample_diploid( r,
	&gametes,
	&diploids,
	&mutations,
	N,
	N,
	&mu[0],
	mmodels,
	recpols,
	&rbw,
	[](gsl_rng * __r, const double __d){ return gsl_ran_poisson(__r,__d); },
	std::bind(KTfwd::insert_at_end<mtype,mlist>,std::placeholders::_1,std::placeholders::_2),
	std::bind(KTfwd::insert_at_end<gtype,glist>,std::placeholders::_1,std::placeholders::_2),
	std::bind(no_selection_multi(),std::placeholders::_1),
	std::bind(KTfwd::mutation_remover(),std::placeholders::_1,0,2*N),
	0.);
~~~

The above call is very similar to the single-locus method, except that vectors of policies are passed.  Two other differences are:

1. We need an array of per-locus mutation rates.  This should be a double * of length \f$L\f$, and &mu[0] passes that pointer on to the mutation policies.
2. We include our Poisson model of interlocus recombination.

At this point, you are probably ready to see the full implementation of  diploid_ind_2locus.cc for the remaining details.

# Sampling from a multilocus simulation

To take a sample of size \f$n \ll N\f$ from the population, you may make a call to either KTfwd::ms_sample, which returns all neutral selected mutations in a single block, or KTfwd::ms_sample_separate, which returns separate blocks for the selected and neutral mutations.

# Serialization: in-memory copying and file I/O

See the [tutorial on serialization](@ref md_md_serialization).

## Mechanics of the multilocus recombination

If anyone is interested in how the book-keeping for multilocus recombination works, see the library file multilocus_rec.hpp, which defines the function KTfwd::fwdpp_internal::multilocus_rec.  You can also look at the unit test code mlocusCrossoverTest.cc, which implements manually-concocted examples to make sure that the outcomes of these functions are correct (which means that I can write down what should happen on paper, and I get the same result from the unit test).
