using NetEFI.Computables;

namespace NetEFI.Design
{
    /// <summary>
    /// Provides the execution context and access to the host application's services (netefi.dll).
    /// </summary>
    public abstract class Context
    {
        /// <summary>
        /// Gets a value indicating whether the user has interrupted the calculation (by pressing Esc in Mathcad).
        /// Useful for long-running calculations in loops.
        /// </summary>
        public abstract bool IsUserInterrupted { get; }

        /// <summary>
        /// Allows calling other registered NetEFI functions by name.
        /// Returns null if no function with that name is found.
        /// </summary>
        /// <param name="functionName">The name of the function to call.</param>
        public abstract IComputable this[ string functionName ] { get; }

        /// <summary>
        /// Checks if a function with the specified name is defined (registered).
        /// </summary>
        /// <param name="functionName">The name of the function.</param>
        public abstract bool IsDefined( string functionName );

        /// <summary>
        /// Writes an informational message to the netefi log file.
        /// </summary>
        public abstract void LogInfo( string text );

        /// <summary>
        /// Writes a formatted informational message to the netefi log file.
        /// </summary>
        public abstract void LogInfo( string format, params object[] args );

        /// <summary>
        /// Writes an error message to the netefi log file.
        /// </summary>
        public abstract void LogError( string text );

        /// <summary>
        /// Writes a formatted error message to the netefi log file.
        /// </summary>
        public abstract void LogError( string format, params object[] args );
    }
}