using NetEFI.Computables;
using NetEFI.Design;
using NetEFI.Runtime;

namespace NetEFI.Functions
{
    /// <summary>
    /// A non-generic base class used by the host to identify all function types. Do not inherit from this directly.
    /// </summary>
    public abstract class MathcadFunctionBase: IComputable
    {
        FunctionInfo IComputable.Info { get; set; }
        FunctionInfo IComputable.GetFunctionInfo( string lang ) => ( ( IComputable ) this ).Info;
        public abstract bool NumericEvaluation( object[] args, out object result, Context context );
    }

    /// <summary>
    /// Base class for a user-defined function with 1 argument.
    /// </summary>
    /// <typeparam name="TArg1">The type of the first argument.</typeparam>
    /// <typeparam name="TResult">The return type of the function.</typeparam>
    public abstract class MathcadFunction<TArg1, TResult>: MathcadFunctionBase
    {
        /// <summary>
        /// Implement your function's logic here.
        /// </summary>
        public abstract TResult Execute( TArg1 arg1, Context context );

        public sealed override bool NumericEvaluation( object[] args, out object result, Context context )
        {
            try
            {
                var arg1 = ( TArg1 ) args[ 0 ];
                result = Execute( arg1, context );
                return true;
            }
            catch ( EFIException ex )
            {
                result = new ErrorResult( ex.ErrNum, ex.ArgNum );
                return true;
            }
        }
    }

    /// <summary>
    /// Base class for a user-defined function with 2 arguments.
    /// </summary>
    public abstract class MathcadFunction<TArg1, TArg2, TResult>: MathcadFunctionBase
    {
        /// <summary>
        /// Implement your function's logic here.
        /// </summary>
        public abstract TResult Execute( TArg1 arg1, TArg2 arg2, Context context );

        public sealed override bool NumericEvaluation( object[] args, out object result, Context context )
        {
            try
            {
                var arg1 = ( TArg1 ) args[ 0 ];
                var arg2 = ( TArg2 ) args[ 1 ];
                result = Execute( arg1, arg2, context );
                return true;
            }
            catch ( EFIException ex )
            {
                result = new ErrorResult( ex.ErrNum, ex.ArgNum );
                return true;
            }
        }
    }

    /// <summary>
    /// Base class for a user-defined function with 3 arguments.
    /// </summary>
    public abstract class MathcadFunction<TArg1, TArg2, TArg3, TResult>: MathcadFunctionBase
    {
        /// <summary>
        /// Implement your function's logic here.
        /// </summary>
        public abstract TResult Execute( TArg1 arg1, TArg2 arg2, TArg3 arg3, Context context );

        public sealed override bool NumericEvaluation( object[] args, out object result, Context context )
        {
            try
            {
                var arg1 = ( TArg1 ) args[ 0 ];
                var arg2 = ( TArg2 ) args[ 1 ];
                var arg3 = ( TArg3 ) args[ 2 ];
                result = Execute( arg1, arg2, arg3, context );
                return true;
            }
            catch ( EFIException ex )
            {
                result = new ErrorResult( ex.ErrNum, ex.ArgNum );
                return true;
            }
        }
    }

    /// <summary>
    /// Base class for a user-defined function with 4 arguments.
    /// </summary>
    public abstract class MathcadFunction<TArg1, TArg2, TArg3, TArg4, TResult>: MathcadFunctionBase
    {
        /// <summary>
        /// Implement your function's logic here.
        /// </summary>
        public abstract TResult Execute( TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, Context context );

        public sealed override bool NumericEvaluation( object[] args, out object result, Context context )
        {
            try
            {
                var arg1 = ( TArg1 ) args[ 0 ];
                var arg2 = ( TArg2 ) args[ 1 ];
                var arg3 = ( TArg3 ) args[ 2 ];
                var arg4 = ( TArg4 ) args[ 3 ];
                result = Execute( arg1, arg2, arg3, arg4, context );
                return true;
            }
            catch ( EFIException ex )
            {
                result = new ErrorResult( ex.ErrNum, ex.ArgNum );
                return true;
            }
        }
    }

    /// <summary>
    /// Base class for a user-defined function with 5 arguments.
    /// </summary>
    public abstract class MathcadFunction<TArg1, TArg2, TArg3, TArg4, TArg5, TResult>: MathcadFunctionBase
    {
        /// <summary>
        /// Implement your function's logic here.
        /// </summary>
        public abstract TResult Execute( TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, Context context );

        public sealed override bool NumericEvaluation( object[] args, out object result, Context context )
        {
            try
            {
                var arg1 = ( TArg1 ) args[ 0 ];
                var arg2 = ( TArg2 ) args[ 1 ];
                var arg3 = ( TArg3 ) args[ 2 ];
                var arg4 = ( TArg4 ) args[ 3 ];
                var arg5 = ( TArg5 ) args[ 4 ];
                result = Execute( arg1, arg2, arg3, arg4, arg5, context );
                return true;
            }
            catch ( EFIException ex )
            {
                result = new ErrorResult( ex.ErrNum, ex.ArgNum );
                return true;
            }
        }
    }

    /// <summary>
    /// Base class for a user-defined function with 6 arguments.
    /// </summary>
    public abstract class MathcadFunction<TArg1, TArg2, TArg3, TArg4, TArg5, TArg6, TResult>: MathcadFunctionBase
    {
        /// <summary>
        /// Implement your function's logic here.
        /// </summary>
        public abstract TResult Execute( TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6, Context context );

        public sealed override bool NumericEvaluation( object[] args, out object result, Context context )
        {
            try
            {
                var arg1 = ( TArg1 ) args[ 0 ];
                var arg2 = ( TArg2 ) args[ 1 ];
                var arg3 = ( TArg3 ) args[ 2 ];
                var arg4 = ( TArg4 ) args[ 3 ];
                var arg5 = ( TArg5 ) args[ 4 ];
                var arg6 = ( TArg6 ) args[ 5 ];
                result = Execute( arg1, arg2, arg3, arg4, arg5, arg6, context );
                return true;
            }
            catch ( EFIException ex )
            {
                result = new ErrorResult( ex.ErrNum, ex.ArgNum );
                return true;
            }
        }
    }

    /// <summary>
    /// Base class for a user-defined function with 7 arguments.
    /// </summary>
    public abstract class MathcadFunction<TArg1, TArg2, TArg3, TArg4, TArg5, TArg6, TArg7, TResult>: MathcadFunctionBase
    {
        /// <summary>
        /// Implement your function's logic here.
        /// </summary>
        public abstract TResult Execute( TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6, TArg7 arg7, Context context );

        public sealed override bool NumericEvaluation( object[] args, out object result, Context context )
        {
            try
            {
                var arg1 = ( TArg1 ) args[ 0 ];
                var arg2 = ( TArg2 ) args[ 1 ];
                var arg3 = ( TArg3 ) args[ 2 ];
                var arg4 = ( TArg4 ) args[ 3 ];
                var arg5 = ( TArg5 ) args[ 4 ];
                var arg6 = ( TArg6 ) args[ 5 ];
                var arg7 = ( TArg7 ) args[ 6 ];
                result = Execute( arg1, arg2, arg3, arg4, arg5, arg6, arg7, context );
                return true;
            }
            catch ( EFIException ex )
            {
                result = new ErrorResult( ex.ErrNum, ex.ArgNum );
                return true;
            }
        }
    }

    /// <summary>
    /// Base class for a user-defined function with 8 arguments.
    /// </summary>
    public abstract class MathcadFunction<TArg1, TArg2, TArg3, TArg4, TArg5, TArg6, TArg7, TArg8, TResult>: MathcadFunctionBase
    {
        /// <summary>
        /// Implement your function's logic here.
        /// </summary>
        public abstract TResult Execute( TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6, TArg7 arg7, TArg8 arg8, Context context );

        public sealed override bool NumericEvaluation( object[] args, out object result, Context context )
        {
            try
            {
                var arg1 = ( TArg1 ) args[ 0 ];
                var arg2 = ( TArg2 ) args[ 1 ];
                var arg3 = ( TArg3 ) args[ 2 ];
                var arg4 = ( TArg4 ) args[ 3 ];
                var arg5 = ( TArg5 ) args[ 4 ];
                var arg6 = ( TArg6 ) args[ 5 ];
                var arg7 = ( TArg7 ) args[ 6 ];
                var arg8 = ( TArg8 ) args[ 7 ];
                result = Execute( arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, context );
                return true;
            }
            catch ( EFIException ex )
            {
                result = new ErrorResult( ex.ErrNum, ex.ArgNum );
                return true;
            }
        }
    }

    /// <summary>
    /// Base class for a user-defined function with 9 arguments.
    /// </summary>
    public abstract class MathcadFunction<TArg1, TArg2, TArg3, TArg4, TArg5, TArg6, TArg7, TArg8, TArg9, TResult>: MathcadFunctionBase
    {
        /// <summary>
        /// Implement your function's logic here.
        /// </summary>
        public abstract TResult Execute( TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6, TArg7 arg7, TArg8 arg8, TArg9 arg9, Context context );

        public sealed override bool NumericEvaluation( object[] args, out object result, Context context )
        {
            try
            {
                var arg1 = ( TArg1 ) args[ 0 ];
                var arg2 = ( TArg2 ) args[ 1 ];
                var arg3 = ( TArg3 ) args[ 2 ];
                var arg4 = ( TArg4 ) args[ 3 ];
                var arg5 = ( TArg5 ) args[ 4 ];
                var arg6 = ( TArg6 ) args[ 5 ];
                var arg7 = ( TArg7 ) args[ 6 ];
                var arg8 = ( TArg8 ) args[ 7 ];
                var arg9 = ( TArg9 ) args[ 8 ];
                result = Execute( arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, context );
                return true;
            }
            catch ( EFIException ex )
            {
                result = new ErrorResult( ex.ErrNum, ex.ArgNum );
                return true;
            }
        }
    }

    /// <summary>
    /// Base class for a user-defined function with 10 arguments.
    /// </summary>
    public abstract class MathcadFunction<TArg1, TArg2, TArg3, TArg4, TArg5, TArg6, TArg7, TArg8, TArg9, TArg10, TResult>: MathcadFunctionBase
    {
        /// <summary>
        /// Implement your function's logic here.
        /// </summary>
        public abstract TResult Execute( TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6, TArg7 arg7, TArg8 arg8, TArg9 arg9, TArg10 arg10, Context context );

        public sealed override bool NumericEvaluation( object[] args, out object result, Context context )
        {
            try
            {
                var arg1 = ( TArg1 ) args[ 0 ];
                var arg2 = ( TArg2 ) args[ 1 ];
                var arg3 = ( TArg3 ) args[ 2 ];
                var arg4 = ( TArg4 ) args[ 3 ];
                var arg5 = ( TArg5 ) args[ 4 ];
                var arg6 = ( TArg6 ) args[ 5 ];
                var arg7 = ( TArg7 ) args[ 6 ];
                var arg8 = ( TArg8 ) args[ 7 ];
                var arg9 = ( TArg9 ) args[ 8 ];
                var arg10 = ( TArg10 ) args[ 9 ];
                result = Execute( arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, context );
                return true;
            }
            catch ( EFIException ex )
            {
                result = new ErrorResult( ex.ErrNum, ex.ArgNum );
                return true;
            }
        }
    }
}