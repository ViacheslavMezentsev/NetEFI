using NetEFI.Computables;
using NetEFI.Design;

public class csecho: IComputable
{
    public FunctionInfo Info => new FunctionInfo( "csecho", "s", "return string", typeof( string ), new[] { typeof( string ) } );

    public FunctionInfo GetFunctionInfo( string lang ) => Info;

    public bool NumericEvaluation( object[] args, out object result, Context context )
    {
        if ( context.IsDefined( "vbecho" ) )
        {
            context[ "vbecho" ].NumericEvaluation( args, out result, context );
        }
        else
        {
            result = Evaluate( ( string ) args[0] );
        }        

        return true;
    }

    public string Evaluate( string text ) => text;
}
