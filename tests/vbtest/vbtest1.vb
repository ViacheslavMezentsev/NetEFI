Imports NetEFI

Public Class vbtest1
    Implements IFunction

    Private _info As FunctionInfo

    Public ReadOnly Property Info() As FunctionInfo _
        Implements IFunction.Info

        Get
            Return _info
        End Get
    End Property

    Public Sub New()

        _info = New FunctionInfo("vbtest1", "x", "return complex scalar 2 * x", _
            GetType(TComplex), _
            New Type() {GetType(TComplex)})
    End Sub

    Public Function GetFunctionInfo(lang As String) As FunctionInfo _
        Implements IFunction.GetFunctionInfo

        Return Info
    End Function

    Public Function NumericEvaluation(args As Object(), ByRef result As Object, ByRef context As Context) As Boolean _
        Implements IFunction.NumericEvaluation

        Try

            Dim arg0 = CType(args(0), TComplex)

            result = New TComplex(2 * arg0.Real, 2 * arg0.Imaginary)

        Catch ex As Exception

            Return False

        End Try

        Return True

    End Function

End Class