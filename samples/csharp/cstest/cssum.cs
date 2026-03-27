using System.Numerics;
using NetEFI.Functions;
using NetEFI.Runtime;

namespace cstest
{
    [Computable( "cssum", "a, b", "Calculates the complex sum of two scalars." )]
    public class CsSum: CustomFunction<Complex, Complex, Complex>
    {
        public override Complex Execute( Complex a, Complex b, Context context ) => a + b;
    }
}