Imports NetEFI


Public Class vbecho
    Implements IFunction

    Public ReadOnly Property Info() As FunctionInfo Implements IFunction.Info

        Get
            Return New FunctionInfo("vbecho", "s", "return string", _
                GetType(String), New Type() {GetType(String)})
        End Get

    End Property

    Public Function GetFunctionInfo(lang As String) As FunctionInfo Implements IFunction.GetFunctionInfo

        Return Info

    End Function

    Public Function NumericEvaluation(args As Object(), ByRef result As Object, ByRef context As Context) As Boolean _
        Implements IFunction.NumericEvaluation

        result = args(0)

        Return True

    End Function

End Class
