using System;
using System.Reflection;

using NetEFI;


public class cstest3: IFunction {

    private FunctionInfo _info;

    public FunctionInfo Info {

        get { return _info; }
    }

    public cstest3() {

        _info = new FunctionInfo(

            "cstest3", "n, m", "return matrix n, m",
            typeof( TComplex[,] ),
            new[] { typeof( TComplex ), typeof( TComplex ) }
            );
    }

    public FunctionInfo GetFunctionInfo( string lang ) {

        return Info;
    }

    public bool NumericEvaluation( object[] args, out object result ) {

        TComplex[,] mat = null;

        result = mat;

        try {

            var n = ( int ) ( ( TComplex ) args[0] ).Real;
            var m = ( int ) ( ( TComplex ) args[1] ).Real;

            mat = new TComplex[n, m];

            for ( var r = 0; r < n; r++ )
                for ( var c = 0; c < m; c++ )
                    mat[r, c] = new TComplex( r, c );

            result = mat;

        } catch ( Exception ex ) {

            return false;
        }

        return true;
    }

}
