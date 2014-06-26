Imports System.Reflection
Imports NetEFI

Public Class vbtest2

    Implements IFunction

    Private _info As FunctionInfo

    Public ReadOnly Property Info() As FunctionInfo Implements IFunction.Info

        Get
            Return _info
        End Get
    End Property

    Public Sub New()

        _info = New FunctionInfo("vbtest2", "separ, v", "return string: v[0] separ v[1] separ ...", _
            New Uri(Assembly.GetExecutingAssembly().CodeBase).LocalPath, _
            GetType(String), _
            New Type() {GetType(String), GetType(TComplex(,))})
    End Sub

    Public Function GetFunctionInfo(ByVal lang As String) As FunctionInfo Implements IFunction.GetFunctionInfo

        Return Info
    End Function

    Public Function NumericEvaluation(ByVal args As Object(), ByRef result As Object) As Boolean Implements IFunction.NumericEvaluation

        Try

            Dim d = DirectCast(args(0), [String])
            Dim v = DirectCast(args(1), TComplex(,))

            Dim len As Integer = v.GetLength(0)

            Dim list = New List(Of String)()

            For n As Integer = 0 To len - 1
                list.Add([String].Format("{0} + {1} * i", v(n, 0).Real, v(n, 0).Imaginary))
            Next

            result = [String].Join(d, list.ToArray())

        Catch ex As Exception

            Return False

        End Try

        Return True

    End Function

End Class
