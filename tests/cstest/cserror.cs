using System.Numerics;

using NetEFI.Computables;
using NetEFI.Design;
using NetEFI.Runtime;

public partial class cserror: IComputable
{
    public static string[] Errors =
    {
        "cserror: text example 1",
        "cserror: text example 2",
        "cserror: text example 3",
        "cserror: text example 4"
    };

    public FunctionInfo Info => new FunctionInfo( "cserror", "n", "return error string", typeof( string ), new[] { typeof( Complex ) } );

    public FunctionInfo GetFunctionInfo( string lang ) => Info;

    public bool NumericEvaluation( object[] args, out object result, Context context )
    {
        throw new EFIException( ( int ) ( ( Complex ) args[0] ).Real, 1 );
    }
}
