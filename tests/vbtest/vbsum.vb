Imports System.Numerics
Imports NetEFI


Public Class vbsum
    Implements IFunction

    Public ReadOnly Property Info() As FunctionInfo Implements IFunction.Info

        Get
            Return New FunctionInfo("vbsum", "a,b", "complex sum of scalars a and b",
                    GetType(Complex),
                    New Type() {GetType(Complex), GetType(Complex)})
        End Get
    End Property

    Public Function GetFunctionInfo(lang As String) As FunctionInfo Implements IFunction.GetFunctionInfo
        Return Info
    End Function

    Public Function NumericEvaluation(args As Object(), ByRef result As Object, ByRef context As Context) As Boolean _
        Implements IFunction.NumericEvaluation

        result = Evaluate( args(0), args(1) )

        Return True
    End Function

    Public Function Evaluate(a As Complex, b As Complex) As Complex

        Return New Complex(a.Real + b.Real, a.Imaginary + b.Imaginary)

    End Function
End Class
