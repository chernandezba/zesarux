/*

SPG compress / uncompress routines from UnrealSpeccy emulator. See file licenses/LICENSE_unrealspeccy

*/

#include "cpu.h"
//------------------- DeHrust --------------------------

//class BBStream
//{
  //private:
	z80_byte* base;
	z80_byte* p;
	int   idx;
	int   len;
	int  eof;
	z80_int  bits;

  //public:

        z80_byte dehrust_getByte( void )
        {
          if( p - base == len ) { eof = 1; return 0; }
          return *p++;
        }

	void BBStream( z80_byte* from, int blockSize )
	{
	  base = p = from;

	  len = blockSize;
	  idx = 0;
	  eof = 0;

	  bits  = dehrust_getByte();
	  bits += 256*dehrust_getByte();
	}


	z80_byte dehrust_getBit()
	{
	  z80_int mask[]  = { 0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100, 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1 };

	  z80_byte bit = ( bits & mask[idx] ) ? 1 : 0;
	  if( idx == 15 )
	  {
		bits  = dehrust_getByte();
		bits += 256*dehrust_getByte();
	  }

	  idx = (idx + 1) % 16;
	  return bit;
	}

	z80_byte dehrust_getBits( int n )
	{
	  z80_byte r = 0;
	  do { r = 2*r + dehrust_getBit(); } while( --n );
	  return r;
	}

	int dehrust_error( void ) { return eof; }
//};


/* depacker */

z80_int dehrust(z80_byte* dst, z80_byte* src, int size)
{
  BBStream(src, size);

  z80_byte *to = dst;

  *to++ = dehrust_getByte();

  z80_byte noBits = 2;
  z80_byte mask[] = { 0, 0, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0 };

  while( !dehrust_error() )
  {
	while (dehrust_getBit())
		*to++ = dehrust_getByte();

	z80_int len = 0;
	z80_byte bb; //  = 0;
	do
	{
	  bb = dehrust_getBits( 2 );
	  len += bb;
	} while( bb == 0x03 && len != 0x0f );

	short offset = 0;

	if( len == 0 )
	{
	  offset = 0xfff8 + dehrust_getBits( 3 );
	  *to = to[offset];
		to++;
	  continue;
	}

	if( len == 1 )
	{
	  z80_byte code = dehrust_getBits(2);

	  if( code == 2 )
	  {
		z80_byte b = dehrust_getByte();
		if( b >= 0xe0 )
		{
		  b <<= 1; ++b; // rlca
		  b ^= 2;	   // xor c

		  if( b == 0xff ) { ++noBits; continue; }

		  offset = 0xff00 + b - 0x0f;

		  *to = to[offset];
			to++;

		  *to = dehrust_getByte();
			to++;

		  *to = to[offset];
			to++;

		  continue;
		}
		offset = 0xff00 + b;
	  }

	  if( code == 0 || code == 1 )
	  {
		offset = dehrust_getByte();
		offset += 256*(code ? 0xfe : 0xfd );
	  }
	  if( code == 3 ) offset = 0xffe0 + dehrust_getBits( 5 );

		z80_byte i;
	  for( i = 0; i < 2; ++i ) {
			*to = to[offset];
			to++;
		}
	  continue;
	}

	if( len == 3 )
	{
	  if( dehrust_getBit() )
	  {
		offset = 0xfff0 + dehrust_getBits( 4 );
		*to = to[offset];
		to++;

		*to = dehrust_getByte();
		to++;

		*to = to[offset];
		to++;

		continue;
	  }

	  if( dehrust_getBit() )
	  {
		z80_byte noBytes = 6 + dehrust_getBits(4);
		z80_byte i;
		for( i = 0; i < 2*noBytes; ++i ) *to++ = dehrust_getByte();
		continue;
	  }

	  len = dehrust_getBits( 7 );
	  if( len == 0x0f )
		  break; // EOF
	  if( len <  0x0f )
		  len = 256*len + dehrust_getByte();
	}

	if( len == 2 ) ++len;

	z80_byte code = dehrust_getBits( 2 );

	if( code == 1 )
	{
	  z80_byte b = dehrust_getByte();

	  if( b >= 0xe0 )
	  {
		if( len > 3 ) return 0;

		b <<= 1; ++b; // rlca
		b ^= 3;	   // xor c

		offset = 0xff00 + b - 0x0f;

		*to = to[offset];
		to++;

		*to = dehrust_getByte();
		to++;

		*to = to[offset];
		to++;

		continue;
	  }
	  offset = 0xff00 + b;
	}

	if( code == 0 ) offset = 0xfe00 + dehrust_getByte();
	if( code == 2 ) offset = 0xffe0 + dehrust_getBits( 5 );
	if( code == 3 )
	{
	  offset  = 256*( mask[noBits] + dehrust_getBits(noBits) );
	  offset += dehrust_getByte();
	}

	z80_int i;
	for( i = 0; i < len; ++i ) {
		*to = to[offset];
		to++;
	}
  }

  return to - dst;
}


//------------------- DeMegaLZ --------------------------


		z80_byte *from;
		z80_byte *to;
		z80_byte bitstream;
		int spg_depack_bitcount;


		void deMLZ(z80_byte *dst, z80_byte *src)
		{
			from = src;
			to = dst;
		}

		z80_byte get_byte(void)
		{
			return *from++;
		}

		void init_bitstream(void)
		{
			bitstream = get_byte();
			spg_depack_bitcount = 8;
		}



		void put_byte(z80_byte val)
		{
			*to++ = val;
		}

		void repeat(z80_long_int disp, int num)
		{
			int i;
			for(i = 0; i < num; i++)
			{
				z80_byte val = *(to - disp);
				put_byte(val);
			}
		}

		// gets specified number of bits from bitstream
		// returns them LSB-aligned
		z80_long_int get_bits(int count)
		{
			z80_long_int bits = 0;

			while (count--)
			{
				if (spg_depack_bitcount--)
				{
					bits <<= 1;
					bits |= 1 & (bitstream >> 7);
					bitstream <<= 1;
				}
				else
				{
					init_bitstream();
					count++;    // repeat loop once more
				}
			}

			return bits;
		}

		int get_bigdisp(void)
		{
			z80_long_int bits;

			// inter displacement
			if (get_bits(1))
			{
				bits = get_bits(4);
				return 0x1100 - (bits << 8) - get_byte();
			}

			// shorter displacement
			else
				return 256 - get_byte();
		}


// depacker
void demlz(z80_byte *dst, z80_byte *src, int size GCC_UNUSED)
{
	deMLZ (dst, src);
	z80_long_int done = 0;
	int i;

	// get first byte of packed file and write to output
	put_byte(get_byte());

	// second byte goes to bitstream
	init_bitstream();

	// actual depacking loop!
	do
	{
		// get 1st bit - either OUTBYTE or beginning of LZ code
		// OUTBYTE
		if (get_bits(1))
			put_byte(get_byte());

		// LZ code
		else
		{
			switch (get_bits(2))
			{
				case 0: // 000
					repeat(8 - get_bits(3), 1);
					break;

				case 1: // 001
					repeat(256 - get_byte(), 2);
					break;

				case 2: // 010
					repeat(get_bigdisp(), 3);
					break;

				case 3: // 011
					// extract num of length bits
					for (i = 1; !get_bits(1); i++);

					// check for exit code
					if (i == 9)
						done = 1;
					else if (i <= 7)
					{
						// get length bits itself
						int bits = get_bits(i);
						repeat(get_bigdisp(), 2 + (1<<i) + bits);
					}
					break;
			}
		}
	} while (!done);
}
