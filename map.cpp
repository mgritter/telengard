#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <map>
#include <utility>
#include <stdlib.h>

typedef unsigned long u32;
typedef unsigned long long u64;

// http://groups.google.com/groups?hl=en&lr=&ie=UTF-8&newwindow=1&selm=5922%40uiucdcs.UUCP
//
// ...
// C64 Basic floating point numbers are broken up into three contiguous
// sections: the exponent (one byte) followed by the mantissa (four bytes)
// followed by the sign of the number (one byte).  I'll go over each of the 
// parts in turn.
//
// The exponent is stored in "excess 128" which means it has a value 128
// higher than the true exponent of the number.  That is, exponents can have a
// value of -128 to 127 but are stored as 0 to 255.  (The purpose of 
// "excess n" is
// to avoid having to deal with negative exponents.  It makes comparisons for
// size much easier.)  What the exponent tells you is how many bits the 
// mantissa must be shifted from its stored value to give you its true value.
// 
// The mantissa is stored in a normalized format, i.e., the binary point is
// assumed to be to the left of the value and the leftmost bit is always a 1.
// Hence the 32 bit values stored here represent numbers between zero and one.
// 
// If the high order bit of the sign byte is on then the number is negative.
// If it is off then the number is positive.
// ...

#if 0
class C64Float {
private:
  int sign_;
  int exponent_; // must be in the range -128 to 127
  u32 mantissa_;

public:
  C64Float( int sign, u32 mantissa, int exponent ) :
    sign_( sign ), exponent_( exponent ), mantissa_( mantissa ) {
  }

  C64Float( int n ) {
    if ( n == 0 ) {
      sign_ = 0;
      exponent_ = 0;
      mantissa_ = 0;
    } else if ( n < 0 ) {
      sign_ = -1; n = -n;
    } else {
      sign_ = 1;
    }
   
    mantissa_ = n;
    exponent_ = 32;
    
    // Normalize
    while ( !( mantissa_ & 0x80000000 ) ) {
      mantissa_ <<= 1;
      --exponent_;
    }
  }

  int asInt( void ) const {
    if ( exponent_ <= 0 ) return 0;
    return ( mantissa_ >> ( 32 - exponent_ ) );
  }

  friend const C64Float 
  operator+( const C64Float &a, const C64Float &b );

  friend const C64Float 
  operator*( const C64Float &a, const C64Float &b );

};

const C64Float 
operator+( const C64Float &a, const C64Float &b ) {
  int xdiff = a.exponent_ - b.exponent_;
  if ( xdiff >= 0 ) {
    u32 am = >>;
    return C64float( a.exponent_);
  } //... TODO
}

const C64Float 
operator*( const C64Float &a, const C64Float &b ) {
  //TODO
}
#endif

// I'll try implementing this using double and truncating the mantissa.
// We don't care about the larger exponent range for this application.
// IEEE has an implicit leading "1.", while C64 has an explicit "0.1",
// so C64 floating point has only 31 effective bits when expressed
// in IEEE format.
// 
//   S EEEEEEEEEEE FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//   0 1        11 12                                                63
//
// C64-limited range:
//   S EEEEEEEEEEE FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000 
//
// The byte masks are:
// 0x    f   f   f    f   f   f   f   f   f   f   e   0   0   0   0   0
//
// Experimentation shows u64 has the same layout as double, so as a gross
// hack we can use the mask 0xffffffffffe0000ll;
//
// This doesn't seem to give exactly the same results.  :(
//

class C64Float {
private:
  double ieeeRep_;

  //void chop() { *(u64 *)&ieeeRep_ &= 0xffffffffffe00000ll; }

  // Chopping at 31 does not seem to give as accurate results
  // as just keeping all the bits.  We are not iterating enough
  // for it to make a difference.
  void chop() { }

public:
  C64Float( double d ) : ieeeRep_( d ) { chop(); }

  double asDouble( void ) const { return ieeeRep_; }
  int asInt( void ) const { return static_cast<int>( ieeeRep_ ); }

  C64Float realPart( void ) const {
    return C64Float( ieeeRep_ - (int)ieeeRep_ ).asDouble();
  }
};


const C64Float 
operator+( const C64Float &a, const C64Float &b ) {
  return C64Float( a.asDouble() + b.asDouble() );
}

const C64Float 
operator*( const C64Float &a, const C64Float &b ) {
  return C64Float( a.asDouble() * b.asDouble() );
}

const C64Float xo = 1.6915;
const C64Float yo = 1.4278;
const C64Float zo = 1.2462;
const C64Float w0 = 4694;


int roomContents( int x, int y, int z ) {
  C64Float q = x*xo + y*yo + z*zo + (x+xo)*(y+yo)*(z+zo);
  int h = ( q.realPart() * w0 ).asInt(); 
#if 0
  std::cerr
    << "x,y,z = " << x << "," << y << "," << z << std::endl
    << "    q = " << std::setprecision(10) << q.asDouble() << std::endl
    << "    h = " << h << std::endl;
#endif
  if ( ( h >> 8 ) > 5 ) { // original is INT(H%/256)>5
    h = h & 0xff;
  }
  if ( ( h >> 8 ) > 0 ) { // original is INT(H%/256)>0
    int newhb = ( ( q * 10 ).realPart() * 15 + 1 ).asInt();
    while ( newhb > 9 ) newhb -= 9; // NOT mod 10.
    h = ( newhb << 8 ) | h & 0xff;
  }

  // Because we do the -= 9 ahead of time, this h will not be exact.
  // std::cerr << "h = " << h << std::endl;

  if ( x == 1 || x == 201 ) h |= 12;
  if ( y == 1 || y == 201 ) h |= 3;

  return h;
}

const char *word1[10] = { "Salty", "Bold", "Loud", "Old", "Goodly",
                          "Worthy", "Lofty", "Fine", "Rocky", "Aged" };
const char *word2[10] = { "Road", "Eye", "Tooth", "Dragon", "Mug",
                          "Demon", "Wharf", "Bridge", "Meade", "Ale" };
const char *word3[10] = { "Tavern", "Alehouse", "Cellar", "Club", "Inn",
                          "House", "Inn", "Lodge", "Meadhall", "Resthouse" };

void 
printRoom( int d ) {
  switch ( d & 3 ) {
  case 2: std::cout << "door(N) "; break;
  case 3: std::cout << "wall(N) "; break;
  }
  switch ( ( d >> 2 ) & 3 ) {
  case 2: std::cout << "door(W) "; break;
  case 3: std::cout << "wall(W) "; break;
  }
  switch ( d >> 8 ) {
  case 1: std::cout << "elevator"; break;
  case 2: std::cout << "pit"; break;
  case 3: std::cout << "teleport"; break;
  case 4: std::cout << "stairs(down)"; break;
  case 5: std::cout << "altar"; break;
  case 6: std::cout << "fountain"; break;
  case 7: std::cout << "cube"; break;
  case 8: std::cout << "throne"; break;
  case 9: std::cout << "box"; break;
  }
}

struct Room {
  int x_;
  int y_;
  int z_;
  int contents_;
  
  Room( int x, int y, int z ) : 
    x_( x ), y_( y ), z_( z ),
    contents_( roomContents( x, y, z ) ) {
  }
  
  bool northWall( void ) const { return ( contents_ & 0x3 ) >= 2; }
  bool northDoor( void ) const { return ( contents_ & 0x3 ) == 2; }

  bool westWall( void ) const { return ( ( contents_ >> 2 ) & 0x3 ) >= 2; }
  bool westDoor( void ) const { return ( ( contents_ >> 2 ) & 0x3 ) == 2; }

  bool inn( void ) const { 
    return ( z_ == 1 ) && ( special() == 1 );
  }

  bool teleport( void ) const { 
    return ( special() == 3 );
  }
  
  int special( void ) const {
    return contents_ >> 8;
  }

  const std::string innName( void ) const {
    std::ostringstream out;
    int i1 = ( x_ * y_ ) % 10;
    int i2 = ( x_ + y_ ) % 10;
    int i3 = ( x_*3 +  y_*7 ) % 10;
    out << word1[ i1 ] << " " << word2[ i2 ] << " " << word3[ i3 ];
    return out.str();
  }

  const std::string teleportLocation( void ) const {
    int nz = z_;
    int nx = x_;
    int ny = y_;
    if ( ( ( nx + ny ) & 1 ) == 0 ) nz -= 1;
    if ( ( ( nx + ny ) & 2 ) == 2 ) nz += 2;
    nx = nx + nz*8 + ny*13;
    ny = ny + nz*6 + nx*17;
    while ( nx > 200 ) nx = nx - 200;
    while ( ny > 200 ) ny = ny - 200;
    if ( nz == 0 ) nz = 1;
    if ( nz > 50 ) nz = 50;
    std::ostringstream out;
    out << nx << "," << ny;
    if ( nz != z_ ) out << " on lvl " << nz;
    return out.str();
  }

  const char * icon( void ) const {
    if ( z_ == 1 && x_ == 25 && y_ == 13 ) return "tile-home.png";
    if ( z_ > 1 ) {
      int upperSpecial = roomContents( x_, y_, z_ - 1 ) >> 8;
      if ( upperSpecial == 4 ) {
        // FIXME: check for bidirectional
        return "tile-up.png";
      } 
    }
    switch ( special() ) {
    case 1: 
      if ( z_ == 1 ) return "tile-up.png";
      return "tile-elevator.png";
    case 2: return "tile-pit.png";
    case 3: return "tile-teleport.png";
    case 4:
      if ( z_ == 50 ) return "tile-blank.png";
      return "tile-down.png";
    case 5: return "tile-altar.png";
    case 6: return "tile-fountain.png";
    case 7: return "tile-cube.png";
    case 8: return "tile-throne.png";
    case 9: return "tile-box.png";
    default: return "tile-blank.png";
    }
  }

  const std::string text( void ) const {
    std::ostringstream out;
    out << x_ << "," << y_ << " ";
    if ( inn() ) {
      out << innName();
    } else if ( teleport() ) {
      out << label() << " to " << teleportLocation() << " (80%)";
    } else {
      out << label();
    } 
    return out.str();
  }

  const std::string label( void ) const {
    if ( z_ > 1 ) {
      int upperSpecial = roomContents( x_, y_, z_ - 1 ) >> 8;
      if ( upperSpecial == 4 ) {
        return "stairs up";
      } 
    }
    switch ( special() ) {
    case 1: 
      if ( z_ == 1 ) return "inn";
      return "elevator";
    case 2: return "pit";
    case 3: return "teleport";
    case 4:
      if ( z_ == 50 ) return "";
      return "stairs down";
    case 5: return "altar";
    case 6: return "fountain";
    case 7: return "cube";
    case 8: return "throne";
    case 9: return "box";
    default: return "";
    }
  }
};

void
printRoom( int x, int y, int z ) {
  int r = roomContents( x, y, z );
  std::cout << std::dec << x << "," << y << "," << z << " ";
  printRoom( r );
  std::cout << std::endl;
}

typedef std::multimap< std::string, std::pair<int,int> > InnIndexMap;

void
printUsage( void ) {
  std::cerr << "Usage: map [level [size [x y]]]" << std::endl;
}

int 
main( int argc, char *argv[] ) {
  int z = 1;
  if ( argc > 1 ) {
    if ( std::string( argv[ 1 ] ) == "--help" ||
         std::string( argv[ 1 ] ) == "-h" ) {
      printUsage();
      return 1;
    }
    z = atoi( argv[ 1 ] );
  } 
  int xStart = 1;
  int xEnd = 200;
  int yStart = 1;
  int yEnd = 200;

  if ( argc > 2 ) {
    int radius = ( atoi( argv[ 2 ] ) - 1 ) / 2;
    int xCenter = 25;
    int yCenter = 13;
    if ( argc > 4 ) {
      xCenter = atoi( argv[ 3 ] );
      yCenter = atoi( argv[ 4 ] );
    }
    xStart = xCenter - radius;
    if ( xStart < 1 ) xStart = 1;
    xEnd = xCenter + radius;
    if ( xEnd > 200 ) xEnd = 200;
    yStart = yCenter - radius;
    if ( yStart < 1 ) yStart = 1;
    yEnd = yCenter + radius;
    if ( yEnd > 200 ) yEnd = 200;
  }
  

  int xSize = xEnd - xStart + 1;
  // int ySize = yEnd - yStart + 1;
  int width = xSize * 20;
  
  InnIndexMap innIndex;

  std::cout << "<HTML><HEAD><TITLE>Telengard Level " << z << "</TITLE></HEAD>\n"
            << "<BODY BGCOLOR=\"white\">\n"
            << "<H1>Telengard Level " << z << "</H1>\n"
            << "<H2>" 
            << xStart << "," << yStart << " - " << xEnd << "," << yEnd
            << "</H2>\n"
            << "<TABLE BORDER=\"0\" CELLPADDING=\"0\" CELLSPACING=\"0\" WIDTH=\"" << width << "\">\n";
    
  for ( int y = yStart; y <= yEnd; ++y ) {
    std::cout << "<TR><!-- prerow " << y << " -->\n";
    // First do the north edge
    for ( int x = xStart; x <= xEnd; ++x ) {
      Room r( x, y, z );
      //
      //    c---r.northWall()---
      //    |
      // r.westWall
      // 
      Room rwest( x - 1, y, z );
      Room rnorth( x, y-1, z );

      int count = 0;
      if ( r.northWall() ) count += 1;
      if ( r.westWall() ) count += 1;
      if ( rwest.northWall() ) count += 1;
      if ( rnorth.westWall() ) count += 1;
      if ( count > 1 ) {
        std::cout << "  <TD><IMG SRC=\"tile-wall-3x3.png\" BORDER=\"0\" WIDTH=\"3\" HEIGHT=\"3\"></TD>\n";
      } else {
        std::cout << "  <TD></TD>\n";
      }
      if ( r.northWall() ) {
        if ( r.northDoor() ) {
          std::cout << "  <TD><IMG SRC=\"tile-door-n.png\" BORDER=\"0\" WIDTH=\"17\" HEIGHT=\"3\"></TD>\n";
        } else {
          std::cout << "  <TD><IMG SRC=\"tile-wall-n.png\" BORDER=\"0\" WIDTH=\"17\" HEIGHT=\"3\"></TD>\n";
        }
      } else {
        std::cout << "  <TD></TD>\n";
      }
    }
    std::cout << "</TR>\n";
    std::cout << "<TR><!-- row " << y << " -->\n";
    for ( int x = xStart; x <= xEnd; ++x ) {
      Room r( x, y, z );
      if ( r.westWall() ) {
        if ( r.westDoor() ) {
          std::cout << "  <TD><IMG SRC=\"tile-door-w.png\" BORDER=\"0\" WIDTH=\"3\" HEIGHT=\"17\"></TD>\n";
        } else {
          std::cout << "  <TD><IMG SRC=\"tile-wall-w.png\" BORDER=\"0\" WIDTH=\"3\" HEIGHT=\"17\"></TD>\n";
        }
      } else {
        std::cout << "  <TD></TD>\n";
      }

      std::cout << "  <TD><A onMouseOver=\"window.status='" 
                << r.text() << "'; return true;\"><IMG SRC=\"" << r.icon() 
                << "\" BORDER=\"0\" WIDTH=\"17\" HEIGHT=\"17\"></A></TD>\n";
      if ( r.inn() ) {
        innIndex.insert( make_pair( r.innName(), std::make_pair( x, y ) ) );
      }
    }
    std::cout << "</TR>\n";
  }
  if ( !innIndex.empty() ) {
    std::cout << "</TABLE><H2>Inns</H2><TABLE>\n";
    for ( InnIndexMap::iterator i = innIndex.begin(); i != innIndex.end(); 
          ++i ) {
      std::cout << "<TR><TD>" << i->first << "</TD><TD>"
                << i->second.first << "," << i->second.second 
                << "</TD></TR>\n";
    }
  }
  std::cout << "</TABLE></BODY></HTML>\n";    
  return 0;
}


