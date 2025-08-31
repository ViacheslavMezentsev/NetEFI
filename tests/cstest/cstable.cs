using System.Numerics;

using NetEFI.Functions;
using NetEFI.Runtime;

namespace cstest
{
    [Computable( "cstable", "seed", "Generates a frequency table of pseudo-random numbers." )]
    public class CsTable: CustomFunction<Complex, Complex[,]>
    {
        // State for the PRNG
        private uint x = 1, y, z, w;

        // A fast Marsaglia's Xorshift pseudo-random number generator.
        private byte NextByte()
        {
            uint t = x ^ ( x << 11 );
            x = y; y = z; z = w;
            w = w ^ ( w >> 19 ) ^ ( t ^ ( t >> 8 ) );

            return ( byte ) ( w & 0xFF );
        }

        public override Complex[,] Execute( Complex seed, Context context )
        {
            // Use the input as a seed for the PRNG
            x = ( uint ) seed.Real;
            y = ( uint ) seed.Imaginary;
            z = x | ( y << 16 );
            w = y | ( x << 16 );

            const int dataSize = 256 * 256;
            const int numClasses = 256 * 256;

            // Generate frequency counts
            var counts = new int[ numClasses ];

            for ( int k = 0; k < dataSize; k++ )
            {
                // Generate a 16-bit random number
                var classId = ( NextByte() << 8 ) | NextByte();
                counts[ classId ]++;
            }

            // Prepare the result matrix
            var res = new Complex[ numClasses, 1 ];

            for ( int k = 0; k < numClasses; k++ )
            {
                res[ k, 0 ] = new Complex( counts[ k ], 0 );
            }

            return res;
        }
    }
}