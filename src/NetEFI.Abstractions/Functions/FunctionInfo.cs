using System;

namespace NetEFI.Functions
{
    /// <summary>
    /// Stores metadata about a user-defined function.
    /// </summary>
    public class FunctionInfo
    {
        /// <summary>
        /// The name of the function as it will be called in Mathcad.
        /// </summary>
        public string Name { get; }

        /// <summary>
        /// A string describing the function's parameters (e.g., "x, y, z").
        /// </summary>
        public string Parameters { get; }

        /// <summary>
        /// A brief description of the function's purpose.
        /// </summary>
        public string Description { get; }

        /// <summary>
        /// The return value type (e.g., typeof(string), typeof(Complex), typeof(Complex[,])).
        /// </summary>
        public Type ReturnType { get; }

        /// <summary>
        /// An array of the function's argument types.
        /// </summary>
        public Type[] ArgTypes { get; }

        /// <summary>
        /// Initializes a new instance of the <see cref="FunctionInfo"/> class.
        /// </summary>
        public FunctionInfo( string name, string parameters, string description, Type returnType, params Type[] argTypes )
        {
            Name = name ?? throw new ArgumentNullException( nameof( name ) );
            Parameters = parameters ?? "";
            Description = description ?? "";
            ReturnType = returnType ?? throw new ArgumentNullException( nameof( returnType ) );
            ArgTypes = argTypes ?? throw new ArgumentNullException( nameof( argTypes ) );
        }
    }
}