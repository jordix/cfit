
#include <cfit/models/gauss.hh>
#include <cfit/math.hh>

#include <cfit/random.hh>

#include <cfit/dataset.hh>

/*
  Definitions of several functions based on the definition of the norm.
                          1       (   ( x - mu )^2  )
  G(x)               = ------- exp( - ------------  )
                        _norm     (     2 sigma^2   )

                                 ____                         upper
                                / pi           (   x - mu    )
  _norm              = sigma   / ----       erf( ---------_  )
                             \/   2            (  sigma \/2  )
                                                              lower
                                 ____
                       sigma    / pi  [        (  x - mu    ) ]
  cdf                = -----   / ---- [ 1 + erf( ---------_ ) ]
                       _norm \/   2   [        (  sigma \/2 ) ]

                                 ____                        min( xmax, upper )
                       sigma    / pi           (  x - mu    )
  area( xmin, xmax ) = -----   / ----       erf( ---------_ )
                       _norm \/   2            (  sigma \/2 )
                                                             max( xmin, lower )
*/


Gauss::Gauss( const Variable& x, const Parameter& mu, const Parameter& sigma )
  : _mu( mu ), _sigma( sigma ), _hasLower( false ), _hasUpper( false ), _lower( 0.0 ), _upper( 0.0 ), _doCache( false ), _cacheIdx( 0 )
{
  push( x );

  push( mu    );
  push( sigma );

  cache();
}



Gauss::Gauss( const Variable& x, const ParameterExpr& mu, const ParameterExpr& sigma )
  : _mu( mu ), _sigma( sigma ), _hasLower( false ), _hasUpper( false ), _lower( 0.0 ), _upper( 0.0 ), _doCache( false ), _cacheIdx( 0 )
{
  push( x );

  push( mu    );
  push( sigma );

  cache();
}



Gauss* Gauss::copy() const
{
  return new Gauss( *this );
}


double Gauss::mu()  const
{
  return _mu.evaluate();
}


double Gauss::sigma() const
{
  return _sigma.evaluate();
}


void Gauss::setLowerLimit( const double& lower )
{
  _hasLower = true;
  _lower    = lower;

  cache();
}


void Gauss::setUpperLimit( const double& upper )
{
  _hasUpper = true;
  _upper    = upper;

  cache();
}


void Gauss::setLimits( const double& lower, const double& upper )
{
  _hasLower = true;
  _hasUpper = true;
  _lower    = lower;
  _upper    = upper;

  cache();
}


void Gauss::unsetLowerLimit()
{
  _hasLower = false;

  cache();
}


void Gauss::unsetUpperLimit()
{
  _hasUpper = false;

  cache();
}


void Gauss::unsetLimits()
{
  _hasLower = false;
  _hasUpper = false;

  cache();
}


void Gauss::cache()
{
  const double& vmu    = mu();
  const double& vsigma = sigma();
  const double& sqrt2  = std::sqrt( 2.0 );

  double argmin = 0.0;
  if ( _hasLower )
    argmin = 1.0 + Math::erf( ( _lower - vmu ) / ( vsigma * sqrt2 ) );

  double argmax = 2.0;
  if ( _hasUpper )
    argmax = 1.0 + Math::erf( ( _upper - vmu ) / ( vsigma * sqrt2 ) );

  const double& factor = vsigma * std::sqrt( M_PI / 2.0 );
  _norm = factor * ( argmax - argmin );
}


// Cache the values of the pdf at every point in the dataset, if the parameters are fixed.
const std::map< unsigned, std::vector< double > > Gauss::cacheReal( const Dataset& data )
{
  // Determine whether the amplitudes should be cached, i.e. only if all their parameters are fixed.
  _doCache = getPar( 0 ).isFixed() && getPar( 1 ).isFixed();

  std::map< unsigned, std::vector< double > > cached;

  if ( ! _doCache )
    return cached;

  // Get an index for the cached complex amplitudes.
  _cacheIdx = _cacheIdxReal++;

  const std::string& varname = getVar( 0 ).name();
  const std::size_t& size = data.size();
  for ( std::size_t entry = 0; entry < size; ++entry )
    cached[ _cacheIdx ].push_back( evaluate( data.value( varname, entry ) ) );

  return cached;
}


const double Gauss::evaluate( const double& x ) const throw( PdfException )
{
  return std::exp( - 0.5 * pow( x - mu(), 2 ) / pow( sigma(), 2 ) ) / _norm;
}


const double Gauss::evaluate( const std::vector< double >& vars ) const throw( PdfException )
{
  return evaluate( vars[ 0 ] );
}


const double Gauss::evaluate( const std::vector< double >&                 vars  ,
                              const std::vector< double >&                 cacheR,
                              const std::vector< std::complex< double > >& cacheC ) const throw( PdfException )
{
  if ( ! _doCache )
    return evaluate( vars );

  return cacheR[ _cacheIdx ];
}


void Gauss::setParExpr()
{
  _mu   .setPars( _parMap );
  _sigma.setPars( _parMap );
}



const double Gauss::area( const double& min, const double& max ) const throw( PdfException )
{
  const double& vmu    = mu();
  const double& vsigma = sigma();
  const double& sqrt2  = std::sqrt( 2.0 );

  const double& xmin = _hasLower ? std::max( min, _lower ) : min;
  const double& xmax = _hasUpper ? std::min( max, _upper ) : max;

  const double& argmin = Math::erf( ( xmin - vmu ) / ( vsigma * sqrt2 ) );
  const double& argmax = Math::erf( ( xmax - vmu ) / ( vsigma * sqrt2 ) );

  const double& factor = vsigma * std::sqrt( M_PI / 2.0 );
  return ( argmax - argmin ) * factor / _norm;
}


const std::map< std::string, double > Gauss::generate() const throw( PdfException )
{
  std::normal_distribution< double > dist( mu(), sigma() );
  std::map< std::string, double > gen;
  gen[ getVar( 0 ).name() ] = dist( Random::engine() );
  return gen;
}

