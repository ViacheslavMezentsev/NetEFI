Imports System.Numerics
Imports NetEFI.Computables
Imports NetEFI.Design

Public Class vbtest1
    Implements IComputable

    Public ReadOnly Property Info() As FunctionInfo Implements IComputable.Info

        Get
            Return New FunctionInfo("vbtest1", "x", "return complex scalar 2 * x",
                GetType(Complex), New Type() {GetType(Complex)})
        End Get

    End Property

    Public Function GetFunctionInfo(lang As String) As FunctionInfo Implements IComputable.GetFunctionInfo

        Return Info

    End Function

    Public Function NumericEvaluation(args As Object(), ByRef result As Object, context As Context) As Boolean _
        Implements IComputable.NumericEvaluation

        result = 2 * CType(args(0), Complex)

        Return True

    End Function

End Class