#ifndef __MINIMIZER_HH__
#define __MINIMIZER_HH__

#include <vector>

#include <Minuit/FCNBase.h>
#include <Minuit/FunctionMinimum.h>

#include <cfit/variable.hh>
#include <cfit/dataset.hh>
#include <cfit/pdfbase.hh>


class Minimizer : public FCNBase
{
protected:
  PdfBase&        _pdf;
  const Dataset&  _data;
  std::map< unsigned, std::vector< double >                 > _cacheR;
  std::map< unsigned, std::vector< std::complex< double > > > _cacheC;

  // Minimizer variation to produce uncertaities at a given number of sigmas.
  //    Notice that, if the user wants n-sigma uncertainties, up = n^2.
  double _up;

public:
  Minimizer( PdfBase& pdf, const Dataset& data )
    : _pdf( pdf ), _data( data ), _up( -1.0 )
  {}

  const PdfBase& pdf() const { return _pdf; }

  // Should be const double, but minuit declares these functions as double.
  double up() const throw( MinimizerException );
  double operator()( const std::vector<double>& par ) const throw( PdfException ) = 0;

  // Setters.
  void setUp( const double& up ) { _up = up; }

  void cache()
  {
    const std::map< unsigned, std::vector< double >                 >& cachedR = _pdf.cacheReal   ( _data );
    const std::map< unsigned, std::vector< std::complex< double > > >& cachedC = _pdf.cacheComplex( _data );

    _cacheR.insert( cachedR.begin(), cachedR.end() );
    _cacheC.insert( cachedC.begin(), cachedC.end() );
  }

  FunctionMinimum minimize() const;
};

#endif
