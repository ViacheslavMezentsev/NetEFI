using System.Numerics;

using NetEFI.Computables;
using NetEFI.Design;
using NetEFI.Functions;

namespace cstest
{
    [Computable( "cstest1", "x", "Returns the complex scalar 2 * x." )]
    public class CsTest1: MathcadFunction<Complex, Complex>
    {
        public override Complex Execute( Complex x, Context context )
        {
            return 2 * x;
        }
    }
}