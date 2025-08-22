using System.Numerics;

using NetEFI.Computables;
using NetEFI.Design;

public class cstest3: IComputable
{
    public FunctionInfo Info => new FunctionInfo( "cstest3", "n, m", "return matrix n, m",
        typeof( Complex[,] ), new[] { typeof( Complex ), typeof( Complex ) } );

    public FunctionInfo GetFunctionInfo( string lang ) => Info;

    public bool NumericEvaluation( object[] args, out object result, Context context )
    {
        Complex[,] mat = null;

        result = mat;

        try
        {
            var n = ( int ) ( ( Complex ) args[0] ).Real;
            var m = ( int ) ( ( Complex ) args[1] ).Real;

            mat = new Complex[n, m];

            for ( var r = 0; r < n; r++ )
                for ( var c = 0; c < m; c++ )
                    mat[r, c] = new Complex( r, c );

            result = mat;
        }
        catch
        {
            return false;
        }

        return true;
    }
}
