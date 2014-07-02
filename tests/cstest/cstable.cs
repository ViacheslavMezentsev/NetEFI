using System;
using NetEFI;
using System.Collections.Generic;


public class cstable: IFunction {

    public FunctionInfo Info {

        get { 
            return new FunctionInfo(

            "cstable", "x", "return table of frequenses n x m",
            typeof( TComplex[,] ),
            new[] {typeof( TComplex ) }
            );
        }
    }

    public FunctionInfo GetFunctionInfo( string lang ) { return Info; }

    // Find minimum  Number in the Array 
    private double Min( double[] value ) {

        var min = value[0];

        for ( var i = 1; i < value.Length; i++ ) {

            if ( value[i] < min ) {

                min = value[i];
            }

        }

        return min;
    }

    // Find maximum  Number in the Array 
    private double Max( double[] value) {

        var max = value[0];

        for ( var i = 1; i < value.Length; i++ ) {
         
            if ( value[i] > max ) {

                max = value[i];
            }

        }

        return max;
    }

    uint x = 1, y, z, w;

    // A faster Marsaglia's Xorshift pseudo-random generator in unsafe C#
    // http://roman.st/Article/Faster-Marsaglia-Xorshift-pseudo-random-generator-in-unsafe-C
    byte NextByte() {

        uint t = x ^ ( x << 11 );

        x = y; y = z; z = w;
        w = w ^ ( w >> 19 ) ^ ( t ^ ( t >> 8 ) );

        return ( byte ) ( w & 0xFF );
    }

    public bool NumericEvaluation( object[] args, out object result, ref Context context ) {

        x = 1;
        y = 0;
        z = 0;
        w = 0;

        var nums = new byte[ 256 * 256 + 8 ];

        int k;

        for ( k = 0; k < 256 * 256 + 8; k++ ) nums[k] = NextByte();

        var list = new List<List<int>>();

        for ( k = 0; k < 256 * 256; k++ ) {

            list.Add( new List<int>() );
        }

        for ( k = 0; k < 256 * 256; k++ ) {

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

        var res = new TComplex[ 256 * 256, 1 ];

        for ( k = 0; k < 256 * 256; k++ ) {

            res[ k, 0 ] = new TComplex( list[k].Count, 0 );
        }

        result = res;

        return true;
    }

}
