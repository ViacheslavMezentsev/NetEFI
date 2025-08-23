using System.Numerics;

using NetEFI.Computables;
using NetEFI.Design;
using NetEFI.Functions;

namespace cstest
{
    [Computable( "csdec", "m", "Decodes a vector of packed integers into a 2-bit grayscale matrix." )]
    public class CsDecoder: MathcadFunction<Complex[,], Complex[,]>
    {
        public override Complex[,] Execute( Complex[,] m, Context context )
        {
            var N = m.GetLength( 0 );
            var bytes = new int[ N ];

            for ( var k = 0; k < N; k++ )
            {
                bytes[ k ] = ( int ) m[ k, 0 ].Real;
            }

            var res = new Complex[ 256, 256 ];

            for ( var n = 0; n < 256; n++ )
            {
                for ( var i = 0; i < 32; i++ )
                {
                    var value = bytes[ n * 32 + i ];
                    // Unpack each 16-bit integer into 8 2-bit values, scaled up to 0-192 range.
                    res[ n, 8 * i + 0 ] = new Complex( ( value & 0x0003 ) << 6, 0 );
                    res[ n, 8 * i + 1 ] = new Complex( ( ( value & 0x000C ) >> 2 ) << 6, 0 );
                    res[ n, 8 * i + 2 ] = new Complex( ( ( value & 0x0030 ) >> 4 ) << 6, 0 );
                    res[ n, 8 * i + 3 ] = new Complex( ( ( value & 0x00C0 ) >> 6 ) << 6, 0 );
                    res[ n, 8 * i + 4 ] = new Complex( ( ( value & 0x0300 ) >> 8 ) << 6, 0 );
                    res[ n, 8 * i + 5 ] = new Complex( ( ( value & 0x0C00 ) >> 10 ) << 6, 0 );
                    res[ n, 8 * i + 6 ] = new Complex( ( ( value & 0x3000 ) >> 12 ) << 6, 0 );
                    res[ n, 8 * i + 7 ] = new Complex( ( ( value & 0xC000 ) >> 14 ) << 6, 0 );
                }
            }
            return res;
        }
    }
}