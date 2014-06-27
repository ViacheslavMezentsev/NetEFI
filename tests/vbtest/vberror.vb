Imports NetEFI


Public Class vberror
    Implements IFunction

    Public ReadOnly Property Info() As FunctionInfo Implements IFunction.Info

        Get
            Return New FunctionInfo("vberror", "n", "return error string", _
                    GetType([String]), New Type() {GetType(TComplex)})
        End Get

    End Property

    Public Function GetFunctionInfo(lang As String) As FunctionInfo Implements IFunction.GetFunctionInfo
        Return Info
    End Function

    Public Function NumericEvaluation(args As Object(), ByRef result As Object) As Boolean _
        Implements IFunction.NumericEvaluation

        Throw New EFIException(CInt(CType(args(0), TComplex).Real), 1)
    End Function

End Class
