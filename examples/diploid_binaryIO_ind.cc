/*
  \include diploid_binaryIO_ind.cc

  Same as diploid_binaryIO.cc, but used individual-based routines
  
  The population is then read back in and compared to what was written out.

  Main point here is to show how the write/read function objects for fwdpp/IO.hpp should be written.

  Also illustrates POSIX file locking via <fcntl.h>, which is super-useful on clusters.

  Example use that runs quickly: ./diploid_binaryIO 1000 10 10 10000 1 index haps.bin $RANDOM
*/
#include <numeric>
#include <functional>
#include <cassert>
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <Sequence/SimData.hpp>
#include <fwdpp/diploid.hh>
#include <fwdpp/sugar/infsites.hpp>
#include <fwdpp/sugar/serialization.hpp>
using mtype = KTfwd::popgenmut;
#define SINGLEPOP_SIM
#include <common_ind.hpp>

using mlist = singlepop_serialized_t::mlist_t;
using glist = singlepop_serialized_t::glist_t;
using mtype = singlepop_serialized_t::mutation_t;
using gtype = singlepop_serialized_t::gamete_t;

int main(int argc, char ** argv)
{
  if (argc != 9)
    {
      std::cerr << "Too few arguments\n"
		<< "Usage: diploid_binaryIO_ind N theta rho ngens replicate_no indexfile hapfile seed\n";
      exit(10);
    } 
  int argument=1;
  const unsigned N = atoi(argv[argument++]);
  const double theta = atof(argv[argument++]);
  const double rho = atof(argv[argument++]);
  const unsigned ngens = atoi(argv[argument++]);
  const unsigned replicate_no = atoi(argv[argument++]);
  const char * indexfile = argv[argument++];
  const char * hapfile = argv[argument++];
  const unsigned seed = atoi(argv[argument++]);

  const double mu = theta/double(4*N);
  const double littler = rho/double(4*N);
  
  std::copy(argv,argv+argc,std::ostream_iterator<char*>(std::cerr," "));
  std::cerr << '\n';
  
  GSLrng r(seed);

  unsigned twoN = 2*N;

  singlepop_serialized_t pop(N);
  unsigned generation;
  double wbar;

  //recombination map is uniform[0,1)  
  std::function<double(void)> recmap = std::bind(gsl_rng_uniform,r);

  for( generation = 0; generation < ngens; ++generation )
    {
      wbar = KTfwd::sample_diploid(r,
				   &pop.gametes, 
				   &pop.diploids,
				   &pop.mutations,
				   N,     
				   mu,   
				   std::bind(KTfwd::infsites(),r,&pop.mut_lookup,generation,
					     mu,0.,[&r](){return gsl_rng_uniform(r);},[](){return 0.;},[](){return 0.;}),
				   std::bind(KTfwd::genetics101(),std::placeholders::_1,std::placeholders::_2,
					       &pop.gametes,
					       littler,
					       r,
					       recmap),
				   std::bind(KTfwd::insert_at_end<mtype,mlist>,std::placeholders::_1,std::placeholders::_2),
				   std::bind(KTfwd::insert_at_end<gtype,glist>,std::placeholders::_1,std::placeholders::_2),
				   std::bind(KTfwd::multiplicative_diploid(),std::placeholders::_1,std::placeholders::_2,2.),
				   std::bind(KTfwd::mutation_remover(),std::placeholders::_1,0,2*N));
      KTfwd::remove_fixed_lost(&pop.mutations,&pop.fixations,&pop.fixation_times,&pop.mut_lookup,generation,twoN);
    }
  std::ostringstream buffer;
      
  KTfwd::write_binary_pop(&pop.gametes,&pop.mutations,&pop.diploids,std::bind(KTfwd::mutation_writer(),std::placeholders::_1,std::placeholders::_2),buffer);

  //establish POSIX file locks for output
  struct flock index_flock, hapfile_flock;
  index_flock.l_type = F_WRLCK;/*Write lock*/
  index_flock.l_whence = SEEK_SET;
  index_flock.l_start = 0;
  index_flock.l_len = 0;/*Lock whole file*/
      
  //lock index file ASAP
  FILE * index_fh = fopen(indexfile,"a");
  int index_fd = fileno(index_fh);
  if ( index_fd == -1 ) 
    { 
      std::cerr << "ERROR: could not open " << indexfile << '\n';
      exit(10);
    }
  if (fcntl(index_fd, F_SETLKW,&index_flock) == -1) 
    {
      std::cerr << "ERROR: could not obtain lock on " << indexfile << '\n';
      exit(10);
    }
      
  hapfile_flock.l_type = F_WRLCK;/*Write lock*/
  hapfile_flock.l_whence = SEEK_SET;
  hapfile_flock.l_start = 0;
  hapfile_flock.l_len = 0;/*Lock whole file*/

  FILE * haps_fh = fopen(hapfile,"a");
  int hapfile_fd = fileno(haps_fh);
      
  if ( hapfile_fd == -1 ) 
    { 
      std::cerr << "ERROR: could not open " << hapfile << '\n';
      exit(10);
    }
  if (fcntl(index_fd, F_SETLKW,&index_flock) == -1) 
    {
      std::cerr << "ERROR: could not obtain lock on " << hapfile << '\n';
      exit(10);
    }

  long int offset = ftell(haps_fh);
  //write the index
  fprintf( index_fh,"%u\t%ld\n",replicate_no,offset );

  //write the buffered haplotype data
  write( hapfile_fd, buffer.str().c_str(), buffer.str().size() );

  //release locks and close files
  index_flock.l_type = F_UNLCK;
  hapfile_flock.l_type = F_UNLCK;

  if (fcntl(hapfile_fd, F_UNLCK,&hapfile_flock) == -1) 
    {
      std::cerr << "ERROR: could not releaselock on " <<  hapfile << '\n';
      exit(10);
    }
  fflush( haps_fh );
  fclose(haps_fh);
      
  if (fcntl(index_fd, F_UNLCK,&index_flock) == -1) 
    {
      std::cerr << "ERROR: could not release lock on " << indexfile << '\n';
      exit(10);
    }
  fflush( index_fh );
  fclose(index_fh);

  //now, read the data back in...
  glist gametes2;
  mlist mutations2;
  singlepop_serialized_t::dipvector_t diploids2;

  std::ifstream in(hapfile,std::ios_base::in|std::ios_base::binary);
  in.seekg(offset);
  KTfwd::read_binary_pop(&gametes2,
			 &mutations2,
			 &diploids2,
			 std::bind(KTfwd::mutation_reader<mtype>(),std::placeholders::_1),
			 in);

  //Now, compare what we wrote to what we read
  std::cout << pop.gametes.size() << ' ' << gametes2.size() << ' ' << pop.mutations.size() << ' ' << mutations2.size() 
	    << ' ' << pop.diploids.size() << ' ' << diploids2.size() << '\n';

  for( unsigned i = 0 ; i < pop.diploids.size() ; ++i )
    {
      std::cout << "Diploid " << i << ":\nWritten:\n";
      for( unsigned j = 0 ; j < pop.diploids[i].first->mutations.size() ; ++j )
	{
	  std::cout << '(' << pop.diploids[i].first->mutations[j]->pos << ','
		    << pop.diploids[i].first->mutations[j]->n << ')';
	}
      std::cout << '\n';
      for( unsigned j = 0 ; j <pop.diploids[i].second->mutations.size() ; ++j )
	{
	  std::cout << '(' << pop.diploids[i].second->mutations[j]->pos << ','
		    << pop.diploids[i].second->mutations[j]->n << ')';
	}
      std::cout << '\n';

      std::cout << "Read:\n";
      for( unsigned j = 0 ; j < diploids2[i].first->mutations.size() ; ++j )
	{
	  std::cout << '(' << diploids2[i].first->mutations[j]->pos << ','
		    << diploids2[i].first->mutations[j]->n << ')';
	}
      std::cout << '\n';
      for( unsigned j = 0 ; j < diploids2[i].second->mutations.size() ; ++j )
	{
	  std::cout << '(' << diploids2[i].second->mutations[j]->pos << ','
		    << diploids2[i].second->mutations[j]->n << ')';
	}
      std::cout << '\n';
    }
}
