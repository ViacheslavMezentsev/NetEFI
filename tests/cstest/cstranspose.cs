using NetEFI;

public class cstranspose: IFunction
{
    public FunctionInfo Info
    {
        get
        {
            return new FunctionInfo( "cstranspose", "X", "returns a transpose of X",
                typeof( TComplex[,] ), new[] { typeof( TComplex[,] ) } );
        }
    }

    public FunctionInfo GetFunctionInfo( string lang ) => Info;

    public bool NumericEvaluation( object[] args, out object result, ref Context context )
    {
        result = Evaluate( ( TComplex[,] ) args[0] );

        return true;
    }

    public TComplex[,] Evaluate( TComplex[,] X )
    {
        var rows = X.GetLength(0);
        var cols = X.GetLength(1);

        var Y = new TComplex[ cols, rows ];

        for ( var r = 0; r < rows; r++ )
            for ( var c = 0; c < cols; c++ )
            {
                Y[ c, r ] = X[ r, c ];
            }

        return Y;
    }
}
