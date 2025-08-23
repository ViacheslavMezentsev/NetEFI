using System.Numerics;

using NetEFI.Computables;
using NetEFI.Design;
using NetEFI.Functions;
using NetEFI.Runtime;

namespace cstest
{
    [Computable( "cserror", "n", "Throws a custom error specified by index n." )]
    public class CsError: MathcadFunction<Complex, string>
    {
        public static string[] Errors =
        {
            "cserror: This is custom error message 1.",
            "cserror: This is custom error message 2.",
            "cserror: This is custom error message 3.",
            "cserror: This is custom error message 4."
        };

        public override string Execute( Complex n, Context context )
        {
            // Throws a custom EFIException.
            throw new EFIException( ( int ) n.Real, 1 );
        }
    }
}