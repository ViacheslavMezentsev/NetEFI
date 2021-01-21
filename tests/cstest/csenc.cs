using System.Numerics;
using NetEFI;

public class csenc: IFunction
{
    public FunctionInfo Info => new FunctionInfo( "csenc", "m", "encoder", typeof( Complex[,] ), new[] { typeof( Complex[,] ) } );

    public FunctionInfo GetFunctionInfo( string lang ) => Info;

    public bool NumericEvaluation( object[] args, out object result, ref Context context )
    {
        var N = ( ( Complex[,] ) args[0] ).GetLength(0);
        var M = ( ( Complex[,] ) args[0] ).GetLength(1);

        var bytes = new byte[ N * M ];

        var k = 0;

        for ( var n = 0; n < N; n++ )

            for ( var m = 0; m < M; m++ )
            {
                bytes[k] = ( byte ) ( ( Complex[,] ) args[0] )[n, m].Real;
                k++;
            }

        var count = N * M / 8;

        var res = new Complex[count, 1];

        for ( k = 0; k < count; k++ )
        {
            var value = bytes[8 * k + 0] >> 6;

            value += ( bytes[8 * k + 1] >> 6 ) << 2;

            value += ( bytes[8 * k + 2] >> 6 ) << 4;

            value += ( bytes[8 * k + 3] >> 6 ) << 6;

            value += ( bytes[8 * k + 4] >> 6 ) << 8;

            value += ( bytes[8 * k + 5] >> 6 ) << 10;

            value += ( bytes[8 * k + 6] >> 6 ) << 12;

            value += ( bytes[8 * k + 7] >> 6 ) << 14;

            res[k,0] = new Complex( value, 0);
        }

        result = res;

        return true;
    }
}
