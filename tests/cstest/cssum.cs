using System;
using NetEFI;


public class cssum: IFunction {

    public FunctionInfo Info {

        get {
            return new FunctionInfo( "cssum", "a,b", "complex sum of scalars a and b",
                typeof( TComplex ), new[] { typeof( TComplex ), typeof( TComplex ) } );
        }
    }

    public FunctionInfo GetFunctionInfo( string lang ) { return Info; }

    public bool NumericEvaluation( object[] args, out object result, ref Context context ) {

        result = Evaluate( ( TComplex ) args[0], ( TComplex ) args[1] );

        return true;
    }

    public TComplex Evaluate( TComplex a, TComplex b ) {

        return new TComplex( a.Real + b.Real, a.Imaginary + b.Imaginary );
    }
}
