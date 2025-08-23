namespace NetEFI.Design
{
    /// <summary>
    /// An internal class used to safely transport error information
    /// from managed code back to the host without throwing exceptions across boundaries.
    /// </summary>
    public sealed class ErrorResult
    {
        public int ErrorCode { get; set; }
        public int ArgumentIndex { get; set; }

        public ErrorResult( int errorCode, int argumentIndex )
        {
            ErrorCode = errorCode;
            ArgumentIndex = argumentIndex;
        }
    }
}