using System.IO;
using NetEFI;


public class csrfile: IFunction
{
    public FunctionInfo Info
    {
        get
        { 
            return new FunctionInfo( "csrfile", "file", "return file content", typeof( string ), new[] { typeof( string ) } );
        }
    }

    public FunctionInfo GetFunctionInfo( string lang ) { return Info; }

    public bool NumericEvaluation( object[] args, out object result, ref Context context )
    {
        var path = ( string ) args[0];

        if ( File.Exists( path ) ) result = File.ReadAllText( path ); else result = "";

        return true;
    }
}
