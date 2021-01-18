using NetEFI;

public partial class cserror: IFunction
{
    public FunctionInfo Info
    {

        get
        {
            return new FunctionInfo( "cserror", "n", "return error string", typeof( string ), new[] { typeof( TComplex ) } );
        }
    }

    public FunctionInfo GetFunctionInfo( string lang ) => Info;

    public bool NumericEvaluation( object[] args, out object result, ref Context context )
    {
        throw new EFIException( ( int ) ( ( TComplex ) args[0] ).Real, 1 );
    }
}
