Imports System.Numerics
Imports NetEFI.Computables
Imports NetEFI.Design


Public Class vbsum
    Implements IComputable

    Public ReadOnly Property Info() As FunctionInfo Implements IComputable.Info

        Get
            Return New FunctionInfo("vbsum", "a,b", "complex sum of scalars a and b",
                    GetType(Complex),
                    New Type() {GetType(Complex), GetType(Complex)})
        End Get
    End Property

    Public Function GetFunctionInfo(lang As String) As FunctionInfo Implements IComputable.GetFunctionInfo
        Return Info
    End Function

    Public Function NumericEvaluation(args As Object(), ByRef result As Object, context As Context) As Boolean _
        Implements IComputable.NumericEvaluation

        result = Evaluate(args(0), args(1))

        Return True
    End Function

    Public Function Evaluate(a As Complex, b As Complex) As Complex

        Return a + b

    End Function

End Class
