using System.Numerics;
using NetEFI;

public partial class cserror: IFunction
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

    public bool NumericEvaluation( object[] args, out object result, ref Context context )
    {
        throw new EFIException( ( int ) ( ( Complex ) args[0] ).Real, 1 );
    }
}
