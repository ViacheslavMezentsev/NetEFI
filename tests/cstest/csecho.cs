using System;
using NetEFI;


public class csecho: IFunction {

    public FunctionInfo Info {

        get { 
            return new FunctionInfo(  "csecho", "s", "return string",
                typeof( String ), new[] { typeof( String ) } );
        }
    }

    public FunctionInfo GetFunctionInfo( string lang ) { return Info; }

    public bool NumericEvaluation( object[] args, out object result ) {

        result = args[0];

        return true;
    }

}
