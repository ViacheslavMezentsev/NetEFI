Imports System.Numerics
Imports System.Windows.Forms
Imports System.Drawing
Imports System.Drawing.Imaging

Imports NetEFI

Namespace Capture

    Public Class capture
        Implements IFunction

        Public img As Bitmap = Nothing

        Public ReadOnly Property Info() As FunctionInfo Implements IFunction.Info

            Get
                Return New FunctionInfo( "capture", "n", "take a snapshot from the usb webcam", _
                    GetType(Complex(,)), New Type() {GetType(Complex)} )
            End Get

        End Property

        Public Function GetFunctionInfo( lang As String ) As FunctionInfo Implements IFunction.GetFunctionInfo

            Return Info

        End Function

        Private Function RGB( matimg As Bitmap ) As Complex(,)

            ' Проверяем формат изображения.
            If ( matimg.PixelFormat <> PixelFormat.Format24bppRgb ) AndAlso ( matimg.PixelFormat <> PixelFormat.Format32bppRgb ) Then

                Throw New Exception( "Wrong image format. Must be 24bpp or 32 bpp." )

            End If

            ' Lock the bitmap's bits.
            Dim bmpData = matimg.LockBits( New Rectangle( 0, 0, matimg.Width, matimg.Height ), ImageLockMode.[ReadOnly], matimg.PixelFormat )

            Dim ptr = bmpData.Scan0

            ' Declare an array to hold the bytes of the bitmap.
            Dim length = bmpData.Stride * matimg.Height

            Dim rgbValues = New Byte( length - 1 ) {}

            ' Copy the RGB values into the array.
            Runtime.InteropServices.Marshal.Copy( ptr, rgbValues, 0, length )

            ' Unlock the bits.
            matimg.UnlockBits(bmpData)

            Dim byteLen = Image.GetPixelFormatSize( bmpData.PixelFormat ) / 8
            Dim height = matimg.Height
            Dim width = matimg.Width

            Dim matrix As Complex(,) = New Complex( height - 1, 3 * width - 1 ) {}

            For y = 0 To height - 1

                ' Since Width property specifies number of pixels in single scan line, 
                ' Stride property contains number of bytes in a scan line. As scan line 
                ' may be aligned on four-bytes boundary, stride may differ from bitmap 
                ' data width multiplied on number of bytes per pixel. That's why you 
                ' should use this property to move to the same position on the next row 
                ' instead of any other methods.   

                Dim strideY = bmpData.Stride * y

                For x = 0 To width - 1

                    Dim index = strideY + x * byteLen

                    ' Порядок определяется форматом Bitmap - [BGR].

                    ' B
                    matrix( y, 2 * width + x ) = New Complex( rgbValues( index ), 0 )
                    index += 1

                    ' G
                    matrix( y, width + x ) = New Complex( rgbValues( index ), 0 )
                    index += 1

                    ' R
                    matrix( y, x ) = New Complex( rgbValues( index ), 0 )

                Next

            Next

            Return matrix

        End Function

    Public Function NumericEvaluation(args As Object(), ByRef result As Object, ByRef context As Context) As Boolean _
        Implements IFunction.NumericEvaluation

            Dim mat As Complex(,)

            Try

                Dim deviceId As Integer = CType(args(0), Complex).Real

                mat = New Complex(0, 0) {}

                mat(0, 0) = New Complex(0, 0)

                result = mat

                Dim formFormat = New FormCapture( deviceId, Me )

                If formFormat.ShowDialog() = DialogResult.OK Then

                    If Not IsNothing( img ) Then result = RGB( img )

                End If

            Catch

            End Try

            Return True

        End Function

    End Class

End Namespace
