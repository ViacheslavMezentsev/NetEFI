using System;
using NetEFI;


public partial class cserror: IFunction {

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
