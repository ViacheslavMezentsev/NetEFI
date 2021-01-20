using NetEFI;


public class csecho: IFunction
{
    public FunctionInfo Info => new FunctionInfo( "csecho", "s", "return string", typeof( string ), new[] { typeof( string ) } );

    public FunctionInfo GetFunctionInfo( string lang ) { return Info; }

    public bool NumericEvaluation( object[] args, out object result, ref Context context )
    {
        if ( context.IsDefined( "vbecho" ) )
        {
            context[ "vbecho" ].NumericEvaluation( args, out result, ref context );
        }
        else
        {
            result = Evaluate( ( string ) args[0] );
        }        

        return true;
    }

    public string Evaluate( string text ) => text;
}
