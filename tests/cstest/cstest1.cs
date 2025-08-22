using System.Numerics;

using NetEFI.Computables;
using NetEFI.Design;

public class cstest1: IComputable
{
    public FunctionInfo Info => new FunctionInfo( "cstest1", "x", "return complex scalar 2 * x",
        typeof( Complex ), new[] { typeof( Complex ) } );

    public FunctionInfo GetFunctionInfo( string lang ) => Info;

    public bool NumericEvaluation( object[] args, out object result, Context context )
    {
        result = 2 * ( Complex ) args[0];

        return true;
    }
}
