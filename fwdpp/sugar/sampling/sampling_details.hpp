#ifndef __FWDPP_SUGAR_SAMPLING_SAMPLING_DETAILS_HPP__
#define __FWDPP_SUGAR_SAMPLING_SAMPLING_DETAILS_HPP__

namespace KTfwd
{
  namespace sugar
  {
    enum class treat_neutral {ALL,NEUTRAL,SELECTED};
  }

  template<typename vec_mutation_t>
  void add_fixations( sample_t & sample,
		      const vec_mutation_t & fixations,
		      const unsigned nsam,
		      const sugar::treat_neutral treat )
  {
    for( const auto & f : fixations)
      {
	if( treat == sugar::treat_neutral::ALL )
	  {
	    sample.emplace_back( f.pos,std::string(nsam,'1') );
	  }
	else if (treat == sugar::treat_neutral::NEUTRAL && f.neutral ) //only add neutral mutations
	  {
	    sample.emplace_back( f.pos,std::string(nsam,'1') );
	  }
	else if (treat == sugar::treat_neutral::SELECTED && !f.neutral ) //only add selected mutations
	  {
	    sample.emplace_back( f.pos,std::string(nsam,'1') );
	  }
      }
  }

  template<typename vec_mutation_t>
  void finish_sample( sample_t & sample, const vec_mutation_t & fixations, const unsigned nsam, const bool removeFixed, const sugar::treat_neutral treat )
  {
    if(! removeFixed )
      {
	add_fixations(sample,fixations,nsam,treat);
      }
    std::sort( sample.begin(),sample.end(),
	       [](const sample_site_t & a,
		  const sample_site_t & b) noexcept {
		 return a.first<b.first;
	       });
  }
  
  //Single-region, single-deme
  template<typename poptype>
  sample_t sample_details( const gsl_rng * r,
			   const poptype & p,
			   const unsigned nsam,
			   const bool removeFixed,
			   std::true_type)
  {
    sample_t rv =  ms_sample(r,p.mutations,p.gametes,p.diploids,nsam,removeFixed);
    finish_sample(rv,p.fixations,nsam,removeFixed,sugar::treat_neutral::ALL);
    return rv;
  }

  //Multi-locus, single-deme
  template<typename poptype>
  sample_t sample_details( const gsl_rng * r,
			   const poptype & p,
			   const unsigned nsam,
			   const bool removeFixed,
			   std::false_type)
  {
    sample_t rv = ms_sample(r,p.mutations,p.gametes,p.diploids,nsam,removeFixed);
    finish_sample(rv,p.fixations,nsam,removeFixed,sugar::treat_neutral::ALL);
    return rv;
  }

  //Single-region, single-deme
  template<typename poptype>
  sep_sample_t sample_sep_details( const gsl_rng * r,
				   const poptype & p,
				   const unsigned nsam,
				   const bool removeFixed,
				   std::true_type)
  {
    sep_sample_t rv = ms_sample_separate(r,p.mutations,p.gametes,p.diploids,nsam,removeFixed);
    finish_sample(rv.first,p.fixations,nsam,removeFixed,sugar::treat_neutral::NEUTRAL);
    finish_sample(rv.second,p.fixations,nsam,removeFixed,sugar::treat_neutral::SELECTED);
    return rv;
  }

  template<typename poptype>
  sample_t sample_details( const poptype & p,
			   const std::vector<unsigned> & individuals,
			   const bool removeFixed,
			   std::true_type)
  {
    sep_sample_t temp = fwdpp_internal::ms_sample_separate_single_deme(p.mutations,p.gametes,p.diploids,individuals,2*individuals.size(),removeFixed);
    auto rv = std::move(temp.first);
    std::move(temp.second.begin(),temp.second.end(),std::back_inserter(rv));
    finish_sample(rv,p.fixations,2*individuals.size(),removeFixed,sugar::treat_neutral::ALL);
    return rv;
  }

  template<typename poptype>
  sample_t sample_details( const poptype & p,
			   const std::vector<unsigned> & individuals,
			   const bool removeFixed,
			   std::false_type)
  {
    sep_sample_t temp = fwdpp_internal::ms_sample_separate_single_deme(p.mutations,p.gamtes,p.diploids,individuals,2*individuals.size(),removeFixed);
    auto rv = std::move(temp.first);
    std::move(temp.second.begin(),temp.second.end(),std::back_inserter(rv));
    finish_sample(rv,p.fixations,2*individuals.size(),removeFixed,sugar::treat_neutral::ALL);
    return rv;
  }
  
  template<typename poptype>
  sep_sample_t sample_sep_details( const poptype & p,
				   const std::vector<unsigned> & individuals,
				   const bool removeFixed,
				   std::true_type)
  {
    sep_sample_t rv = fwdpp_internal::ms_sample_separate_single_deme(p.mutations,p.gametes,p.diploids,individuals,2*individuals.size(),removeFixed);
    finish_sample(rv.first,p.fixations,2*individuals.size(),removeFixed,sugar::treat_neutral::NEUTRAL);
    finish_sample(rv.second,p.fixations,2*individuals.size(),removeFixed,sugar::treat_neutral::SELECTED);
    return rv;
  }

  //Multi-locus, single-deme
  template<typename poptype>
  sep_sample_t sample_sep_details( const gsl_rng * r,
				   const poptype & p,
				   const unsigned nsam,
				   const bool removeFixed,
				   std::false_type)
  {
    sep_sample_t rv =  ms_sample_separate(r,p.mutations,p.gametes,p.diploids,nsam,removeFixed);
    finish_sample(rv.first,p.fixations,nsam,removeFixed,sugar::treat_neutral::NEUTRAL);
    finish_sample(rv.second,p.fixations,nsam,removeFixed,sugar::treat_neutral::SELECTED);
    return rv;
  }
}

#endif
