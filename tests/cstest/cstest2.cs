using System.Numerics;
using System.Text;

using NetEFI.Functions;
using NetEFI.Runtime;

namespace cstest
{
    [Computable( "cstest2", "separator, v", "Returns a string representation of a vector with a separator." )]
    public class CsTest2: CustomFunction<string, Complex[,], string>
    {
        public override string Execute( string separator, Complex[,] v, Context context )
        {
            if ( v == null || v.GetLength( 0 ) == 0 )
            {
                return "";
            }

            var sb = new StringBuilder();

            for ( var i = 0; i < v.GetLength( 0 ); i++ )
            {
                // Assuming a column vector
                sb.Append( v[ i, 0 ].ToString() );

                if ( i < v.GetLength( 0 ) - 1 )
                {
                    sb.Append( separator );
                }
            }

            return sb.ToString();
        }
    }
}