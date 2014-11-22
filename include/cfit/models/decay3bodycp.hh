#ifndef __DECAY3BODYCP_HH__
#define __DECAY3BODYCP_HH__

#include <vector>
#include <map>

#include <cfit/decaymodel.hh>
#include <cfit/variable.hh>
#include <cfit/amplitude.hh>
#include <cfit/phasespace.hh>
#include <cfit/function.hh>

#include <Minuit/FunctionMinimum.h>


class Dataset;

class Decay3BodyCP : public PdfModel
{
private:
  Amplitude _amp;

  PhaseSpace _ps;

  bool      _hasKappa;
  Parameter _kappa;

  CoefExpr  _z;

  // Constants to speed up norm calculation.
  double                 _nDir;
  double                 _nCnj;
  std::complex< double > _nXed;
  double                 _norm;

  bool                   _fixedAmp;

  // Maximum value of the pdf.
  double _maxPdf;

  // Indices of the cached direct and conjugated amplitudes.
  bool     _cacheAmps;
  unsigned _ampDirCache;
  unsigned _ampCnjCache;


  std::vector< Function > _funcs;

  const double evaluateFuncs() const;
  const double evaluateFuncs( const double& mSq12, const double& mSq13, const double& mSq23 ) const;
  const double evaluateFuncs( const double& mSq12, const double& mSq13                      ) const;

  // Auxiliary function to compute the center of a bin.
  static const double binCenter( const unsigned& bin, const unsigned& nbins, const double& min, const double& max )
  {
    return ( max - min ) / double( nbins ) * ( bin + 0.5 ) + min;
  }

  const double evaluateUnnorm( const double& mSq12, const double& mSq13, const double& mSq23 ) const throw( PdfException );

  const std::map< unsigned, std::vector< std::complex< double > > > cacheComplex( const Dataset& data );

public:
  Decay3BodyCP( const Variable&   mSq12         ,
                const Variable&   mSq13         ,
                const Variable&   mSq23         ,
                const Amplitude&  amp           ,
                const CoefExpr&   z             ,
                const PhaseSpace& ps            ,
                bool              docache = true  );

  Decay3BodyCP( const Variable&   mSq12         ,
                const Variable&   mSq13         ,
                const Variable&   mSq23         ,
                const Amplitude&  amp           ,
                const CoefExpr&   z             ,
                const Parameter&  kappa         ,
                const PhaseSpace& ps            ,
                bool              docache = true  );

  Decay3BodyCP* copy() const;

  const std::string mSq12name() const { return getVar( 0 ).name(); }
  const std::string mSq13name() const { return getVar( 1 ).name(); }
  const std::string mSq23name() const { return getVar( 2 ).name(); }

  const double mSq12() const { return getVar( 0 ).value(); }
  const double mSq13() const { return getVar( 1 ).value(); }
  const double mSq23() const { return getVar( 2 ).value(); }

  // Norm components getters.
  const double&                 nDir() const { return _nDir; }
  const double&                 nCnj() const { return _nCnj; }
  const std::complex< double >& nXed() const { return _nXed; }

  // Need to define own setters, since function variables and parameters may need to be set, too.
  void setVars( const std::vector< double >&              vars ) throw( PdfException );
  void setVars( const std::map< std::string, Variable >&  vars ) throw( PdfException );
  void setVars( const std::map< std::string, double >&    vars ) throw( PdfException );
  void setPars( const std::vector< double >&              pars ) throw( PdfException );
  void setPars( const std::map< std::string, Parameter >& pars ) throw( PdfException );
  void setPars( const FunctionMinimum&                    min  ) throw( PdfException );

  // Norm components setters.
  void setNormComponents( const double& nDir, const double& nCnj, const std::complex< double >& nXed )
  {
    _fixedAmp = _amp.isFixed();

    if ( _fixedAmp )
    {
      _nDir = nDir;
      _nCnj = nCnj;
      _nXed = nXed;
    }
  }

  void setNormComponents( const double& nDir, const std::complex< double >& nXed )
  {
    _fixedAmp = _amp.isFixed();

    if ( _fixedAmp )
    {
      _nDir = nDir;
      _nCnj = nDir;
      _nXed = nXed;
    }
  }


  void cache();
  const double evaluate(                                                               ) const throw( PdfException );
  const double evaluate( const double& mSq12, const double& mSq13, const double& mSq23 ) const throw( PdfException );
  const double evaluate( const double& mSq12, const double& mSq13                      ) const throw( PdfException );
  const double evaluate( const std::vector< double >& vars                             ) const throw( PdfException );

  const double evaluate( const std::vector< double >&                 vars  ,
                         const std::vector< double >&                 cacheR,
                         const std::vector< std::complex< double > >& cacheC           ) const throw( PdfException );

  const double project ( const std::string& varName, const double& value ) const throw( PdfException );

  void setMaxPdf( const double& max ) { _maxPdf = max; }
  const std::map< std::string, double > generate() const throw( PdfException );

  friend const Decay3BodyCP  operator* (       Decay3BodyCP left, const Function&    right );
  friend const Decay3BodyCP  operator* ( const Function&    left,       Decay3BodyCP right );
  const        Decay3BodyCP& operator*=( const Function& right ) throw( PdfException );
};

#endif