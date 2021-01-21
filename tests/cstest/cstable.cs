using System.Numerics;
using System.Collections.Generic;
using NetEFI;

public class cstable: IFunction
{
    public FunctionInfo Info => new FunctionInfo( "cstable", "x", "return table of frequenses n x m",
        typeof( Complex[,] ), new[] { typeof( Complex ) } );

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

    public bool NumericEvaluation( object[] args, out object result, ref Context context )
    {
        x = 1;
        y = 0;
        z = 0;
        w = 0;

        var nums = new byte[ 256 * 256 + 8 ];

        int k;

        for ( k = 0; k < 256 * 256 + 8; k++ ) nums[k] = NextByte();

        var list = new List<List<int>>();

        for ( k = 0; k < 256 * 256; k++ )
        {
            list.Add( new List<int>() );
        }

        for ( k = 0; k < 256 * 256; k++ )
        {
            var classid = nums[k + 0] >> 6;

            classid += ( nums[k + 1] >> 6 ) << 2;

            classid += ( nums[k + 2] >> 6 ) << 4;

            classid += ( nums[k + 3] >> 6 ) << 6;

            classid += ( nums[k + 4] >> 6 ) << 8;

            classid += ( nums[k + 5] >> 6 ) << 10;

            classid += ( nums[k + 6] >> 6 ) << 12;

            classid += ( nums[k + 7] >> 6 ) << 14;

            list[ classid ].Add( k );
        }

        var res = new Complex[ 256 * 256, 1 ];

        for ( k = 0; k < 256 * 256; k++ )
        {
            res[ k, 0 ] = new Complex( list[k].Count, 0 );
        }

        result = res;

        return true;
    }
}
