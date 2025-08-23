using NetEFI.Computables;
using NetEFI.Design;
using NetEFI.Functions;

namespace cstest
{
    [Computable( "csecho", "s", "Returns the input string." )]
    public class CsEcho: MathcadFunction<string, string>
    {
        public override string Execute( string s, Context context )
        {
            // This demonstrates how to call another function registered with NetEFI.
            if ( context.IsDefined( "vbecho" ) )
            {
                // Create arguments for the target function
                var args = new object[] { s };
                object result;

                // Call vbecho and return its result
                context[ "vbecho" ].NumericEvaluation( args, out result, context );
                return ( string ) result;
            }

            return s; // Fallback if vbecho is not available
        }
    }
}