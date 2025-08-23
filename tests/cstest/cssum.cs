using System.Numerics;

using NetEFI.Computables;
using NetEFI.Design;
using NetEFI.Functions;

namespace cstest
{
    [Computable( "cssum", "a, b", "Calculates the complex sum of two scalars." )]
    public class CsSum: MathcadFunction<Complex, Complex, Complex>
    {
        public override Complex Execute( Complex a, Complex b, Context context )
        {
            return a + b;
        }
    }
}