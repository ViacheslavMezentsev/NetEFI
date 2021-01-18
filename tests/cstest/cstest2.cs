using System.Collections.Generic;

using NetEFI;

public class cstest2: IFunction
{
    public FunctionInfo Info { get; }

    public cstest2()
    {
        Info = new FunctionInfo(
            "cstest2", "separ, v", "return string: v[0] separ v[1] separ ...",
            typeof( string ),
            new[] { typeof( string ), typeof( TComplex[,] ) }
            );
    }

    public FunctionInfo GetFunctionInfo( string lang ) => Info;

    public bool NumericEvaluation( object[] args, out object result, ref Context context )
    {
        try
        {
            var d = ( string ) args[0];
            var v = ( TComplex[,] ) args[1];

            var len = v.GetLength( 0 );

            var list = new List<string>();

            for ( var n = 0; n < len; n++ ) list.Add( string.Format( "{0} + {1} * i", v[n, 0].Real, v[n, 0].Imaginary ) );

            result = string.Join( d, list.ToArray() );
        }
        catch
        {
            result = null;
            return false;
        }

        return true;
    }
}
