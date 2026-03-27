namespace NetEFI.Runtime
{
    /// <summary>
    /// An internal class used to safely transport error information
    /// from managed C# code back to the C++/CLI host without throwing exceptions across the boundary.
    /// This class is created by the CustomFunction base class when an EFIException is caught.
    /// </summary>
    public sealed class ErrorResult
    {
        /// <summary>
        /// The unique key of the error, corresponding to a key in an [Error] attribute.
        /// The host will use this key to find the correct 1-based error index.
        /// </summary>
        public string ErrorKey { get; }

        /// <summary>
        /// The 1-based index of the function argument that caused the error (or 0 for a general error).
        /// </summary>
        public int ArgumentIndex { get; }

        public ErrorResult( string errorKey, int argumentIndex )
        {
            ErrorKey = errorKey;
            ArgumentIndex = argumentIndex;
        }
    }
}