using System;
using NetEFI;


public class cserror: IFunction {

    public static String[] Errors = { "cserror 1", "cserror 2", "cserror 3", "cserror 4" };

    public FunctionInfo Info {

        get {
            return new FunctionInfo( "cserror", "n", "return error string",
                typeof( String ), new[] { typeof( TComplex ) } );
        }
    }

    public FunctionInfo GetFunctionInfo( string lang ) { return Info; }

    public bool NumericEvaluation( object[] args, out object result ) {

        throw new EFIException( ( int ) ( ( TComplex ) args[0] ).Real, 1 );
    }

}
