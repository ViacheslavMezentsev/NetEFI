Imports System.Numerics
Imports NetEFI

Public Class vbtest1
    Implements IFunction

    Public ReadOnly Property Info() As FunctionInfo Implements IFunction.Info

        Get
            Return New FunctionInfo("vbtest1", "x", "return complex scalar 2 * x", _
                GetType(Complex), New Type() {GetType(Complex)})
        End Get

    End Property

    Public Function GetFunctionInfo( lang As String ) As FunctionInfo Implements IFunction.GetFunctionInfo

        Return Info

    End Function

    Public Function NumericEvaluation( args As Object(), ByRef result As Object, ByRef context As Context ) As Boolean _
        Implements IFunction.NumericEvaluation

        result = 2 * CType( args(0), Complex )

        Return True

    End Function

End Class