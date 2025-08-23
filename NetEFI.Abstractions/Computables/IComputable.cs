using NetEFI.Design;

namespace NetEFI.Computables
{
    /// <summary>
    /// Defines the contract for a class that implements a user-defined function for Mathcad.
    /// </summary>
    public interface IComputable
    {
        /// <summary>
        /// Gets the full information about the function required for its registration.
        /// </summary>
        FunctionInfo Info { get; set; }

        /// <summary>
        /// (Optional) Gets localized information about the function.
        /// If not overridden, it should return the value of the Info property.
        /// </summary>
        /// <param name="lang">A three-letter language code (e.g., "rus", "eng").</param>
        FunctionInfo GetFunctionInfo( string lang );

        /// <summary>
        /// The main method for performing the numerical calculation.
        /// </summary>
        /// <param name="args">An array of input arguments from the host application.</param>
        /// <param name="result">An output parameter to return the calculation result.</param>
        /// <param name="context">The execution context, providing access to host services.</param>
        /// <returns>True on successful evaluation, otherwise false.</returns>
        bool NumericEvaluation( object[] args, out object result, Context context );
    }
}