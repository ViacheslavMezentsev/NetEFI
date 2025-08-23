using System.Numerics;

using NetEFI.Computables;
using NetEFI.Design;
using NetEFI.Functions;

namespace cstest
{
    [Computable( "csenc", "m", "Encodes a grayscale matrix into a vector of packed integers." )]
    public class CsEncoder: MathcadFunction<Complex[,], Complex[,]>
    {
        public override Complex[,] Execute( Complex[,] matrix, Context context )
        {
            var N = matrix.GetLength( 0 );
            var M = matrix.GetLength( 1 );
            var bytes = new byte[ N * M ];
            int k = 0;

            for ( var n = 0; n < N; n++ )
            {
                for ( var m = 0; m < M; m++ )
                {
                    bytes[ k ] = ( byte ) matrix[ n, m ].Real;
                    k++;
                }
            }

            var count = N * M / 8;
            var res = new Complex[ count, 1 ];

            for ( k = 0; k < count; k++ )
            {
                // Pack the top 2 bits of 8 consecutive bytes into one 16-bit integer.
                var value = ( bytes[ 8 * k + 0 ] >> 6 );
                value += ( bytes[ 8 * k + 1 ] >> 6 ) << 2;
                value += ( bytes[ 8 * k + 2 ] >> 6 ) << 4;
                value += ( bytes[ 8 * k + 3 ] >> 6 ) << 6;
                value += ( bytes[ 8 * k + 4 ] >> 6 ) << 8;
                value += ( bytes[ 8 * k + 5 ] >> 6 ) << 10;
                value += ( bytes[ 8 * k + 6 ] >> 6 ) << 12;
                value += ( bytes[ 8 * k + 7 ] >> 6 ) << 14;
                res[ k, 0 ] = new Complex( value, 0 );
            }
            return res;
        }
    }
}