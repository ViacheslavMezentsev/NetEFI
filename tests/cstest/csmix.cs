using System.Numerics;

using NetEFI.Computables;
using NetEFI.Design;

public class csmix: IComputable
{
    public FunctionInfo Info => new FunctionInfo( "csmix", "m, direction", "return mixed array",
                typeof( Complex[,] ), new[] { typeof( Complex[,] ), typeof( Complex ) } );

    public FunctionInfo GetFunctionInfo( string lang ) => Info;

    uint x = 1, y, z, w;

    // A faster Marsaglia's Xorshift pseudo-random generator in unsafe C#
    // http://roman.st/Article/Faster-Marsaglia-Xorshift-pseudo-random-generator-in-unsafe-C
    byte NextByte()
    {
        uint t = x ^ ( x << 11 );

        x = y; y = z; z = w;
        w = w ^ ( w >> 19 ) ^ ( t ^ ( t >> 8 ) );

        return ( byte ) ( w & 0xFF );
    }

    public bool NumericEvaluation( object[] args, out object result, Context context )
    {
        var N = ( ( Complex[,] ) args[0] ).GetLength(0);
        var M = ( ( Complex[,] ) args[0] ).GetLength(1);

        var d = ( byte ) ( ( Complex ) args[1] ).Real;

        var res = ( Complex[,] ) args[0];
            
        try
        {
            x = 1;
            y = 0;
            z = 0;
            w = 0;

            var nums = new byte[ N * M + 1 ];

            int k;

            for ( k = 0; k < N * M + 1; k++ ) nums[k] = NextByte();

            byte i, j;
            Complex tmp;

            if ( d == 0 )
            {
                k = 0;

                for ( var n = 0; n < N; n++ )
                {
                    for ( var m = 0; m < M; m++ )
                    {
                        i = nums[ k ];
                        j = nums[ k + 1 ];

                        tmp = res[ n, m ];
                        res[ n, m ] = res[ i, j ];
                        res[ i, j ] = tmp;

                        k++;
                    }
                }
            }
            else
            {
                k = 0;

                for ( var n = N - 1; n >=0 ; n-- )
                {
                    for ( var m = M - 1; m >= 0 ; m-- )
                    {
                        i = nums[ N * M - ( k + 1 ) ];
                        j = nums[ N * M - k ];

                        tmp = res[ n, m ];
                        res[ n, m ] = res[ i, j ];
                        res[ i, j ] = tmp;

                        k++;
                    }
                }
            }
        }
        catch { }

        result = res;

        return true;
    }
}
