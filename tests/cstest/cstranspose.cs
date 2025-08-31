using System.Numerics;

using NetEFI.Functions;
using NetEFI.Runtime;

namespace cstest
{
    [Computable( "cstranspose", "X", "Returns the transpose of matrix X." )]
    public class CsTranspose: CustomFunction<Complex[,], Complex[,]>
    {
        public override Complex[,] Execute( Complex[,] X, Context context )
        {
            var rows = X.GetLength( 0 );
            var cols = X.GetLength( 1 );

            var Y = new Complex[ cols, rows ];

            for ( var r = 0; r < rows; r++ )
            {
                for ( var c = 0; c < cols; c++ )
                {
                    Y[ c, r ] = X[ r, c ];
                }
            }

            return Y;
        }
    }
}