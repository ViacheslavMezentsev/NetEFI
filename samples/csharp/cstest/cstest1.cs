using System.Numerics;
using NetEFI.Functions;
using NetEFI.Runtime;

namespace cstest
{
    [Computable( "cstest1", "x", "Returns the complex scalar 2 * x." )]
    public class CsTest1: CustomFunction<Complex, Complex>
    {
        public override Complex Execute( Complex x, Context context ) => 2 * x;
    }
}