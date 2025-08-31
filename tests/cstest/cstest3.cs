using System.Numerics;

using NetEFI.Functions;
using NetEFI.Runtime;

namespace cstest
{
    // 1. Define the possible errors for this function using attributes.
    [Error( "NonInteger", "Dimensions n and m must be integers." )]
    [Error( "NonPositive", "Dimensions n and m must be positive." )]

    // 2. The function's main description remains in the Computable attribute.
    [Computable( "cstest3", "n, m", "Returns an n x m matrix of complex numbers." )]
    public class CsTest3: CustomFunction<Complex, Complex, Complex[,]>
    {
        public override Complex[,] Execute( Complex nComplex, Complex mComplex, Context context )
        {
            // 3. Check if the inputs have imaginary parts, which would make them non-integers.
            if ( nComplex.Imaginary != 0 || mComplex.Imaginary != 0 )
            {
                // Throw the error using its unique key. The second parameter is the
                // argument index to highlight (0 means the function name itself).
                throw new EFIException( "NonInteger", 0 );
            }

            var n = ( int ) nComplex.Real;
            var m = ( int ) mComplex.Real;

            // 4. Check if the dimensions are positive.
            if ( n <= 0 || m <= 0 )
            {
                // This is another distinct error condition.
                throw new EFIException( "NonPositive", 0 );
            }

            // This part of the code is executed only if all checks pass.
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