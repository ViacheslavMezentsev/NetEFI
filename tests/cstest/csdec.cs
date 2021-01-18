using NetEFI;

public class csdec: IFunction
{
    public FunctionInfo Info
    {
        get
        {
            return new FunctionInfo( "csdec", "m", "decoder",
                typeof( TComplex[,] ), new[] { typeof( TComplex[,] ) } );
        }
    }

    public FunctionInfo GetFunctionInfo( string lang ) => Info;

    public bool NumericEvaluation( object[] args, out object result, ref Context context )
    {
        var N = ( ( TComplex[,] ) args[0] ).GetLength(0);

        var bytes = new int[N];

        for ( var k = 0; k < N; k++ ) bytes[k] = ( int ) ( ( TComplex[,] ) args[0] )[k, 0].Real;

        var res = new TComplex[256, 256];

        for ( var n = 0; n < 256; n++ )
            for ( var m = 0; m < 32; m++ )
            {
                var value  = bytes[ n * 32 + m ];

                res[ n, 8 * m + 0 ] = new TComplex( ( value & 0x0003 ) << 6, 0 );
                res[ n, 8 * m + 1 ] = new TComplex( ( ( value & 0x000C ) >> 2 ) << 6, 0 );
                res[ n, 8 * m + 2 ] = new TComplex( ( ( value & 0x0030 ) >> 4 ) << 6, 0 );
                res[ n, 8 * m + 3 ] = new TComplex( ( ( value & 0x00C0 ) >> 6 ) << 6, 0 );
                res[ n, 8 * m + 4 ] = new TComplex( ( ( value & 0x0300 ) >> 8 ) << 6, 0 );
                res[ n, 8 * m + 5 ] = new TComplex( ( ( value & 0x0C00 ) >> 10 ) << 6, 0 );
                res[ n, 8 * m + 6 ] = new TComplex( ( ( value & 0x3000 ) >> 12 ) << 6, 0 );
                res[ n, 8 * m + 7 ] = new TComplex( ( ( value & 0xC000 ) >> 14 ) << 6, 0 );
            }

        result = res;

        return true;
    }
}
