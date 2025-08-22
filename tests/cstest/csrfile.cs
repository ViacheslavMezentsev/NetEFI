using System.IO;

using NetEFI.Computables;
using NetEFI.Design;

public class csrfile: IComputable
{
    public FunctionInfo Info => new FunctionInfo( "csrfile", "file", "return file content", typeof( string ), new[] { typeof( string ) } );

    public FunctionInfo GetFunctionInfo( string lang ) { return Info; }

    public bool NumericEvaluation( object[] args, out object result, Context context )
    {
        result = "";

        var path = ( string ) args[0];

        if ( File.Exists( path ) ) result = File.ReadAllText( path );

        return true;
    }
}
