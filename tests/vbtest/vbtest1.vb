Imports System.Reflection
Imports NetEFI

Public Class vbtest1
    Implements IFunction

    Private _info As FunctionInfo

    Public ReadOnly Property Info() As FunctionInfo Implements IFunction.Info

        Get
            Return _info
        End Get
    End Property

    Public Sub New()

        _info = New FunctionInfo("vbtest1", "x", "return complex scalar 2 * x", _
            New Uri(Assembly.GetExecutingAssembly().CodeBase).LocalPath, _
            GetType(TComplex), _
            New Type() {GetType(TComplex)})
    End Sub

    Public Function GetFunctionInfo(ByVal lang As String) As FunctionInfo Implements IFunction.GetFunctionInfo

        Return Info
    End Function

    Public Function NumericEvaluation(ByVal args As Object(), ByRef result As Object) As Boolean Implements IFunction.NumericEvaluation

        Dim arg0 = DirectCast(args(0), TComplex)

        result = New TComplex(2 * arg0.Real, 2 * arg0.Imaginary)

        Return True
    End Function

End Class