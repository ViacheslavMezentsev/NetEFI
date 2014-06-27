using System;
using System.Reflection;

using NetEFI;


public class cstest1: IFunction {

    private FunctionInfo _info;

    public FunctionInfo Info {

        get { return _info; }
    }

    public cstest1() {

        _info = new FunctionInfo(

            "cstest1", "x", "return complex scalar 2 * x",
            typeof( TComplex ),
            new[] { typeof( TComplex ) }
            );
    }

    public FunctionInfo GetFunctionInfo( string lang ) {

        return Info;
    }

    public bool NumericEvaluation( object[] args, out object result ) {

        var arg0 = ( TComplex ) args[0];

        result = new TComplex( 2 * arg0.Real, 2 * arg0.Imaginary );

        return true;
    }

}
