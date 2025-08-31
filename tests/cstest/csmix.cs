using System;
using System.Numerics;

using NetEFI.Functions;
using NetEFI.Runtime;

namespace cstest
{
    [Computable( "csmix", "m, direction", "Shuffles the elements of a matrix using a pseudo-random generator." )]
    public class CsMix: CustomFunction<Complex[,], Complex, Complex[,]>
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

        public override Complex[,] Execute( Complex[,] m, Complex direction, Context context )
        {
            // The function modifies the input matrix in place, so we clone it first
            // to avoid side effects for the user in Mathcad.
            var res = ( Complex[,] ) m.Clone();
            var N = res.GetLength( 0 );
            var M = res.GetLength( 1 );
            var d = ( byte ) direction.Real;

            try
            {
                // Reset PRNG state for deterministic shuffling
                x = 1; y = ( uint ) ( N * M ); z = ( uint ) d; w = ( uint ) ( N + M );

                // Pre-generate random indices
                var randomRows = new byte[ N * M ];
                var randomCols = new byte[ N * M ];

                for ( int k = 0; k < N * M; k++ )
                {
                    randomRows[ k ] = ( byte ) ( NextByte() % N );
                    randomCols[ k ] = ( byte ) ( NextByte() % M );
                }

                if ( d == 0 ) // Forward shuffle
                {
                    int k = 0;

                    for ( var n = 0; n < N; n++ )
                    {
                        for ( var i = 0; i < M; i++ )
                        {
                            var tmp = res[ n, i ];
                            res[ n, i ] = res[ randomRows[ k ], randomCols[ k ] ];
                            res[ randomRows[ k ], randomCols[ k ] ] = tmp;
                            k++;
                        }
                    }
                }
                else // Backward (un-shuffle) - requires a different logic not implemented here, so we just reverse the loop
                {
                    int k = N * M - 1;

                    for ( var n = N - 1; n >= 0; n-- )
                    {
                        for ( var i = M - 1; i >= 0; i-- )
                        {
                            var tmp = res[ n, i ];
                            res[ n, i ] = res[ randomRows[ k ], randomCols[ k ] ];
                            res[ randomRows[ k ], randomCols[ k ] ] = tmp;
                            k--;
                        }
                    }
                }
            }
            catch ( Exception ex )
            {
                context.LogError( $"csmix failed: {ex.Message}" );
            }

            return res;
        }
    }
}