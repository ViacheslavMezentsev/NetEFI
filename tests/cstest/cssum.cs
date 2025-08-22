using System.Numerics;

using NetEFI.Computables;
using NetEFI.Design;

public class cssum: IComputable
{
    public FunctionInfo Info => new FunctionInfo( "cssum", "a,b", "complex sum of scalars a and b",
                typeof( Complex ), new[] { typeof( Complex ), typeof( Complex ) } );

    public FunctionInfo GetFunctionInfo( string lang ) => Info;

    public bool NumericEvaluation( object[] args, out object result, Context context )
    {
        result = ( Complex ) args[0] + ( Complex ) args[1];

        return true;
    }
}
