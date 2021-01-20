Imports System.Numerics
Imports NetEFI


Public Class vberror
    Implements IFunction

    Public ReadOnly Property Info() As FunctionInfo Implements IFunction.Info

        Get
            Return New FunctionInfo("vberror", "n", "return error string", _
                    GetType(String), New Type() {GetType(Complex)})
        End Get

    End Property

    Public Function GetFunctionInfo(lang As String) As FunctionInfo Implements IFunction.GetFunctionInfo
        Return Info
    End Function

    Public Function NumericEvaluation(args As Object(), ByRef result As Object, ByRef context As Context) As Boolean _
        Implements IFunction.NumericEvaluation

        Throw New EFIException(CType(args(0), Complex).Real, 1)
    End Function

End Class
