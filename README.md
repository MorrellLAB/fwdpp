	fwdpp - A C++ template library for forward-time population genetic simulations



  Copyright (C) 2013 Kevin Thornton

  fwdpp is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Comments are welcome.

	- Kevin Thornton <krthornt@uci.edu>

#Introduction

fwdpp is a C++ template library that abstracts the basic operations required to implement forward-time simulations of population- and quantitative-genetic models.  The library allows the simulation of single populations or metapopulations evolving under the standard evolutionary forces of drift, recombination, migration, and natural selection.  Arbitrary population size changes are also allowed. Different populations in a metapopulation may evolve under different fitness schemes.

The library uses advanced C++ techniques to allow arbitrary models to be implemented via the implementation of simple policies (see Documentation section below).  A programmer wishing to use the library will need a strong background in templates, function objects, and the Standard Template Library (STL).  Web resources for these topics vary too much in quality to recommend any particular one.  However, there are several classic books that are must-reads for C++ programmers (old school, I know):

1.  Scott Meyer's "trilogy" of "Effective C++", "More Effective C++", and "Effective STL".
2.  Nicolai Josuttis' "The C++ Standard Template Library"
3.  David Vandevoorde and and Nicolai Josuttis, "C++ Templates"

The first two are excellent books for people already familiar with C++ syntax but want to know more about effective software design using the language. Meyer's books are particularly good, espectially the first two.  The C++ Templates book is a bible of how to get the most out of templates.  It is a very advanced and detailed book, but I've found it helpful over the years.

The library user will also need some familiarity with the [boost](http://www.boost.org) libraries, especially "bind" and "function".  I refer the user to the boost website for the relevant documentation.

##A note about which version to use

This code is distributed via my gitub [account](http://www.github.com/molpopgen).  The "master" and "dev" branches should be viewed as experimental.  The [releases](https://github.com/molpopgen/fwdpp/releases), however, correspond to tested versions of the library fit for public consumption.  This means that, while the version number in the configure script on master/dev may match that of a recent release, _that does not mean that the features/stability/bugs present in master/dev are identical to those of the release._  If you want to use fwdpp for research, use the latest [release](https://github.com/molpopgen/fwdpp/releases).  If you want to play around with the latest and (occasionally not-so) greatest, look at the dev branch.  If you want to look at the latest I believe to be stable, look at master.  Also note that master may be ahead of dev, etc., depending on what I've committed from my development server to the repo stored at github.

##Which C++?

fwdpp does not use any features from then newly-released C++11 standard.  The new standard extends/simplifies the language, and therefore I expect the current code base to be C++11-compliant. As compiler support for C++11 becomes more widespread, the library will likely start to use some of those features, which will drastically improve readability of some of the nastier bits of template wizardry.

##Citation

The manuscript describing fwdpp is currently on [arxiv](http://arxiv.org/abs/1401.3786).

#Dependencies

fwdpp depends upon the following libraries:

[boost](http://www.boost.org)<br>
[GSL](http://gnu.org/software/gsl)<br>
[zlib](http://zlib.net)<br>
[libsequence](http://github.com/molpopgen/libsequence)<br>

The first three are  available as pre-built packages on most Linux distributions.  The latter (libsequence) also depends on the first three, and must be built from source.

#Installation

./configure<br>
make<br>
make install<br>

##If dependent libraries are in non-stanard locations.

For example, if libsequence is in /opt:

CXXFLAGS=-I/opt/include LDFLAGS="$LDFLAGS -L/opt/lib" ./configure<br>
make<br>
make install

##Installing in a custom location

./configure --prefix=/path/2/where/you/want it

For example:

./configure --prefix=$HOME

##"Fast" installation into a user's $HOME folder

If you do not have permission for anything other than a local install, there is a [script](https://gist.github.com/molpopgen/9160680) that will install gsl-1.1.6, boost 1.5.5, and the master branch of libsequence into your home directory.  This means that header files end up in $HOME/include and run-time libraries end up in $HOME/lib.  (Typically, the "should" be in /usr/local instead of $HOME, but you may not have the permissions to install there).

First, get libsequence and its dependencies all installed:

> git clone https://gist.github.com/molpopgen/9160680<br>
> cd 9160680<br>
> (At this point, edit the script if you are an OS X user)<br>
> bash libseq _ local.sh<br>

The above takes a while.

Now, you can get fwdpp installed.  Below, we cover how to install both the master branch or a stable release into $HOME.

Once installed, you should see the directory $HOME/include/fwdpp, and it should be full of header files.  Nothing goes in $HOME/lib.

To get the master branch from git:

> git clone https://github.com/molpopgen/fwdpp<br>
> cd fwdpp<br>
> CXXFLAGS=-I$HOME/include LDFLAGS=-L$HOME/lib ./configure --prefix=$HOME<br>
> make install

Then, you can compile the example programs:

> cd examples<br>
> LDFLAGS=-I$HOME/lib make

See below for how to run the examples.

If you want a stable release of fwdpp (rather than the master branch which may be semi-experimental), click on [releases](https://github.com/molpopgen/fwdpp/releases) from the main [project page](https://github.com/molpopgen/fwdpp) at github.

On a decent browser, when you click on a release, it should be called fwdpp-version.tar.ga.  Sometimes, though, you may get version.tar.gz.  This is a browser-by-github interaction problem.  On my systems, I get the correct result.

Then,

> tar xzf fwdpp-version.tar.gz<br>
> tar xzf fwdpp-version<br>
> CXXFLAGS=-I$HOME/include LDFLAGS=-L$HOME/lib ./configure<br>
> make install<br>

And now you can compile the examples as described above.


##Examples

The best documentation of how to use the library are the example simulations in the examples/ subdirectory.

###Compiling the examples:

To compile the example code, please make sure that all of the above dependencies are installed correctly. Correctly means that they are installed and any system variables such as LDFLAGS, LD\_LIBRARY\_PATH, CPPFLAGS, etc., are updated to reflect any non-standard installation locations on your system. A non-standard location means anywhere other than /usr or /usr/local, and includes your user's home directory. If you need help with this, please contact your local computer support people.

To compile the examples:

tar xzf fwdpp-version.tar.gz<br>
cd fwdpp-version/examples<br>
make<br>

See the fwdpp documentation for the source code for these examples. 

Note:

These programs are meant to illustrate the programming features of fwdpp. They are NOT necessarily appropriate for research use!!!!!

###Running the examples:

This following list of parameters and their definitions is in common to all of the example programs:<br>
N = the number of diploid individuals to simulate<br>
theta = 4Nu, where u is the mutation rate per gamete per generation. For example, theta = 100 means that on average, 2Nu = 50 new mutations are entering the population each generation.<br>
rho = 4Nr, where r is the recombination rate per diploid per generation<br>
g = the number of generations to simulate. Often, this should be >= 8N at a minimum<br>
n = the sample size to draw at the end of the simulation. To match the typical modeling assumptions of population genetics, you should have n << 2N.<br>
nreps = the number of replicates to simulate<br>
seed = a random number seed. I will use $RANDOM as a seed, referring to the bash shell method to return a random integer.<br>

Note: familiarity with Hudson's "[ms](http://home.uchicago.edu/~rhudson1)" program is helpful for some of what comes below.

####diploid

The first program, diploid, simulates a Wright-Fisher population with mutation, recombination, and drift. To run it:

./diploid N theta rho g n nreps seed | gzip > outfile.gz

The data in outfile.gz will be in the same format as Dick Hudson's "ms" coalescent simulator.

Example:

./diploid 10000 10 10 100000 50 1 $RANDOM | gzip > test\_diploid.gz

####diploid\_ind

Identical to diploids, but uses the individual-based sampler. As this program (and diploid) simulate neutral models, this program will typically be slower than that gamete-based implementation when N is large.

####diploid\_binaryIO

The next program is called diploid\_binaryIO. This program is identical to diploid, except that it only simulates one replicate at a time, creates two output files, and outputs the entire population rather than a sample of size n << 2N. The first file is an index file, containing an integer representing the replicate number of the output and the position in the haplotypes file where this record begins. The second file is the haplotypes file, which contains the entire population in binary format. 

Usage:

./diploid\_binaryIO N theta rho g replicate\_number index\_filename haplotype\_filename seed.

Example for Open Grid Engine compute clusters:

\#!sh<br>
\#$ -t 1-100<br>
\#$ -N DIPBIN<br>

seed=`` `echo "$SGE_TASK_ID*$RANDOM"|bc -l` ``

\#note: the below assumes that the binary is in the users's $PATH and that the GE system knows how to link it to the relevant dynamic libraries

diploid\_binaryIO 10000 10 10 100000 $SGE\_TASK\_ID indexfile hapfile $seed

The above script, when submitted to a Grid Engine queue, will result in 100 populations of size N=10,000 being written to hapfile. Further, “indexfile” will contain the ID number and position of each file. Records are not over-written because the program uses POSIX file locking to ensure that only 1 process at a time can do the writing. This is a complex program, as it mixes C++ objects with output streams such that they can be written to C-style file descriptors, which is required in order to use file locking (which is a C feature with no C++ analog). However, the advantage is that you write all data to one large file, avoiding the plague of lots of small files that can bring distributed file systems to their knees.

####diploid\_binaryIO\_ind

Identical to diploid\_binaryIO, but individual-based.

####diploid\_fixed\_sh

This program is similar to diploid, but adds an additional mutation rate (theta\_selected = 4Nu\_s, where u\_s is the mutation rate per gamete per generation to mutations with selection coefficient s) to mutations with selection coefficient s and dominance h. Fitness across sites is multiplicative. The output is in "ms" format--one block for neutral mutations followed by one block for selected mutations.

Usage:

./dipoid\_fixed\_sh N theta theta\_selected rho s h g n nreps seed | gzip > outfile.gz

For this program, s can be positive or negative, as can h.

####diploid\_fixed\_sh\_ind

Identical to diploid\_fixed\_sh, but based on the individual-based sampler.

####diploid\_twopop\_mig

This program simulates an ancestral population for g generations, at which point a daughter population of size N is “budded” off from the ancestral population. Evolution continues for g2 more generations, with symmetric migration at rate M = 4Nm, where m is the migration rate per diploid per generation. The output is in "ms" format, with n haplotypes per sample just like how ms outputs data for multi-population models.

Usage:

./diploid\_twopop\_mig N theta rho g g2 M n nreps seed | gzip > outfile.gz

Note: the demographic model here implemented may be viewed as biologically bizarre, as it mimics the default behavior of “ms” for population split models. Let’s compare a specific example vs. the equivalent ms command line:

./diplod\_twopop\_mig 10000 50 50 100000 1000 1 50 1 $RANDOM | gzip > outfile.gz

and

ms 100 1 -t 50 -r 50 1000 -I 2 2 1 -ej 0.025 2 1 -em 0.025 1 2 0.

Why may this be considered odd? In ms, when two populations are merged, the rate of coalescence is unaffected by default (this behavior is documented, and it is up to the user to adjust population sizes when population merge and split in ms). That means when the two populations, each of size N merge, the merged (ancestral) population is still of size N. diploid\_twopop\_mig is doing the same thing forwards in time: an ancestral population of size N magically changes into two populations of size N. 

####migsel\_ind

Simulates 2 equal-sized populations of size N (N remains constant over time) diploids with selection at strength s and dominance h. Migration occurs between the two populations.

_Note that the two populations never share a common ancestor in this simulation!  In other words, deme 1 is never founded by a sampling event from deme 0.  This means that if you simulate for a short period of time, it is unlikely that there is a MRCA common to all individuals sampled.  (See comment above about appropriateness of these examples for research...)_

Usage:

./migsel\_ind N 4Nu\_neut 4Nu\_sel 4Nr 4Nm s h f1 f2 ngens n outfilename seed

where:
N = population number for each deme.
4Nu\_neut = 4N times the neutral mutation rate per gamete
4Nu\_sel = 4N times the mutation rate per gamete to selected mutations
4Nr = 4N times the recombination rate per diploid per region per generation
4Nm = 4N times the probability of migration per diploid per generation
s = the selection coefficient of newly-arising selected mutations
h = dominance of newly-arising selected mutations
f1 and f2 are the probabilities of inbreeding in deme 1 and 2, respectively
ngens = # of generations to simulate
n = sample size to take from each deme @ end of simulation (must be even #)
out filename = name of output file
seed = random number seed.

Notes: s is taken to be s in deme 1 and -s jn deme 2. This let’s me illustrate how different fitness functions can be passed to different demes using fwdpp.

The output file contains the following:

1. The metapopulation in binary format
2. Two "ms"-format blocks of size 2*n. These are for neutral and selected mutations, respectively. Within each block, the first n haplotypes are from deme 1 and the second n are from deme 2.

The program writes the data to the output file and then reads it in again. This is mainly to illustrate the binary I/O routines for individual-based metapopulation simulations.

####RHH

This program simulates the "recurrent hitch-hiking" model in which the expected number of fixations of beneficial codominant mutations per site per 4N generations is Lambda. The output is ms-format (just one block for neutral mutations). This model is one of the most thoroughly-studied models of the effect of selection on linked variation. See, for example:

1. Wiehe, T. H. and W. Stephan, 1993 Analysis of a genetic hitchhiking model, and its application to DNA polymorphism data from Drosophila melanogaster. Molecular Biology and Evolution 10: 842–854.
2. Kaplan, N. L., R. R. Hudson and C. H. Langley, 1989 The "hitchhiking effect" revisited. Genetics 123: 887–899.
4. Braverman, J. M., R. R. Hudson, N. L. Kaplan, C. H. Langley and W. Stephan, 1995 The hitchhiking effect on the site frequency spectrum of DNA polymorphisms. Genetics 140: 783–796.
4. Przeworski, M., 2002 The Signature of Positive Selection at Randomly Chosen Loci. Genetics 160: 1179.

The model in this program allows selected mutations within the region with neutral mutation rate theta (the so-called "neutral" or "sampled" region in the coalescent literature treating RHH via the structured coalescent). In general, the majority of selected sites will appear up to s/r\_bp base pairs, where r is the recombination rate per base pair.

The population is simulated for g generations with only neutral mutations, followed by g2 more generations with neutral and selected mutations. The selection coefficient, s, must be > 0, but the program doesn't check for this, so just enter an appropriate value please. The output is in ms format, and only contains data for neutral mutations, so any segregating selected mutations will not be output. The region with mutation rate theta is L base pairs long. L is really only used to calculate r\_bp, which is rho/(4*N*(L-1)), as there are L-1 possible positions for recombination to occur in a region of L nucleotides.

Usage:

./RHH N theta rho L s Lambda g1 g2 n nreps seed | gzip > outfile.gz

Note: many of the well-known formulas for the effect of RHH on linked, neutral variation make very strong assumptions about the parameter values. For example, N needs to be large and s needs to be small, but Ns needs to be large. Further, Lambda needs to be sufficiently small such that sweeps are independent in time. This means that plugging in values to this program and comparing to theoretical predictions may lead to apparent discrepancies. This is also the case with various coalescent simulations of RHH.

####bneck\_selection

This program simulates a population for g generations at size N. In generation g+1, N changes to N2 <= N. The population then grows exponentially to size N3 >= N2 in g2 generations. Selected and neutral mutations are allowed each generation. The output is in “ms” format--one block for neutral mutations followed by one block for selected mutations.

Usage:

./bneck\_selection N theta\_neutral theta\_sel rho s h g1 N2 N3 g2 n nreps seed

Where:<br>
N = starting population size<br>
theta\_neutral = 4N*(neutral mutation rate per gamete)<br>
theta\_sel = 4N*(mutation rate per gamete to selected mutations)<br>
rho = 4Nr, where r is recombination rate per diploid per generation<br>
s = selection coefficient. Can be negative or positive.<br>
h = dominance of selected mutations<br>
N2 = size of bottlenecked population<br>
N3 = size of recovered population<br>
g2 = generations taken to go from size N2 to size N3<br>
n = sample size to take from the population.<br>
nreps = # replicates to simulate<br>
seed = random number seed.<br>

#Documentation

The documentation is in the doc subdirectory that comes with the library.  There are two major pieces of documentation.  First is the detailed documentation of all library functions.  This is generated via [doxygen](http://www.doxygen.org), and the output is a folder called html.  To view the documentation, point a browser to html/index.html.  

The second piece of documentation is a tutorial on writing policies conforming to what fwdpp expects.  This document is doc/policies.tex and a pdf file of the documentation may be obtained by processing the file as follows:

cd doc<br>
pdflatex policies<br>
pdflatex policies<br>

One runs pdflatex twice to ensure that cross-references within the document are processed properly.