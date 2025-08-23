using System.Numerics;

using NetEFI.Computables;
using NetEFI.Design;
using NetEFI.Functions;

namespace cstest
{
    [Computable( "cstest3", "n, m", "Returns an n x m matrix of complex numbers." )]
    public class CsTest3: MathcadFunction<Complex, Complex, Complex[,]>
    {
        public override Complex[,] Execute( Complex nComplex, Complex mComplex, Context context )
        {
            var n = ( int ) nComplex.Real;
            var m = ( int ) mComplex.Real;

            if ( n <= 0 || m <= 0 )
            {
                return new Complex[ 0, 0 ]; // Return empty matrix for invalid dimensions
            }

            var matrix = new Complex[ n, m ];
            for ( var r = 0; r < n; r++ )
            {
                for ( var c = 0; c < m; c++ )
                {
                    matrix[ r, c ] = new Complex( r, c );
                }
            }
            return matrix;
        }
    }
}