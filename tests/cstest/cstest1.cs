using System.Numerics;
using NetEFI;

public class cstest1: IFunction
{
    public FunctionInfo Info => new FunctionInfo( "cstest1", "x", "return complex scalar 2 * x",
        typeof( Complex ), new[] { typeof( Complex ) } );

    public FunctionInfo GetFunctionInfo( string lang ) => Info;

    public bool NumericEvaluation( object[] args, out object result, ref Context context )
    {
        result = 2 * ( Complex ) args[0];

        return true;
    }
}
