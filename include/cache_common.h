#include <iostream>
#include <string>
#include <fstream>

using namespace std;

typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#define LW_SIZE 32
#define DW_SIZE 64

#define BIT_MASK( data, bit )                          ( (data) & (1<<(bit)) )
#define BIT_EXT( data, bit )                           ( (BIT_MASK( data, bit ) >> (bit)) & 1 )
#define FIELD_MASK( msb, lsb )                         ( (~0 << (lsb)) & ((unsigned int)~0 >> (LW_SIZE-(msb)-1)) )
#define FIELD_EXT( data, msb, lsb )                    ( ((data) >> (lsb)) & ~(~((uint64_t) 0) << ((msb)-(lsb)+1)) )
#define BIT_INSERT( word, data_bit, bit )              { if ( data_bit ) word |= (1<<(bit)); else word &= ~(1<<(bit)); }
#define FIELD_INSERT( word, data, msb, lsb )           { word &= ~FIELD_MASK( msb, lsb ); word |= (data) << (lsb); }


#define BIT_MASK_64( data, bit )                       ( (data) & ((uint64_t)1<<(bit)) )
#define BIT_EXT_64( data, bit )                        ( (BIT_MASK_64( data, bit ) >> (bit)) & (uint64_t)1 )
#define FIELD_MASK_64_1( msb, lsb )                    (~((uint64_t)0) << (lsb))
#define FIELD_MASK_64_2( msb, lsb )                    (~((uint64_t)0) >> (DW_SIZE-(msb)-1))
#define FIELD_MASK_64( msb, lsb )                      ( (~((uint64_t)0) << (lsb)) & (~((uint64_t)0) >> (DW_SIZE-(msb)-1)) )
#define FIELD_EXT_64( data, msb, lsb )                 ( ((data) >> (lsb)) & ~(~((uint64_t) 0) << ((msb)-(lsb)+1)) )


#define BIT_INSERT_64( word, data_bit, bit )           { if ( data_bit ) word |= ((uint64_t)1<<(bit)); else word &= ~((uint64_t)1<<(bit)); }
#define FIELD_INSERT_64( word, data, msb, lsb )        { word &= ~FIELD_MASK_64( msb, lsb ); word |= (data) << (lsb); }
