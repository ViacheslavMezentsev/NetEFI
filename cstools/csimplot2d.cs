using System.Linq;
using System.Collections.Generic;
using System.Numerics;

using NetEFI;

/// <summary>
/// Represtents a point in two-dimensional space. Used for representation
/// of points world coordinates.
/// </summary>
internal struct PointD
{
    /// <summary>
    /// X-Coordinate of the point.
    /// </summary>
    public double X;

    /// <summary>
    /// Y-Coordinate of the point.
    /// </summary>
    public double Y;

    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="x">X-Coordinate of the point.</param>
    /// <param name="y">Y-Coordinate of the point.</param>
    public PointD( double x, double y )
    {
        X = x;
        Y = y;
    }

    /// <summary>
    /// returns a string representation of the point.
    /// </summary>
    /// <returns>string representation of the point.</returns>
    public override string ToString()
    {
        return X + ", " + Y;
    }
}

public class csimplot2d : IFunction
{
    #region Private fields

    private readonly byte[] _edgeTable = { 0, 9, 3, 10, 6, 15, 5, 12, 12, 5, 15, 6, 10, 3, 9, 0 };

    private readonly List<int[][]> _table = new List<int[][]>()
        {
            new[] { new int[0] },
            new[] { new[] { 0, 3 } },
            new[] { new[] { 0, 1 } },
            new[] { new[] { 1, 3 } },
            new[] { new[] { 1, 2 } },
            new[] { new[] { 0, 3 }, new [] { 1, 2 } },
            new[] { new[] { 0, 2 } },
            new[] { new[] { 2, 3 } },
            new[] { new[] { 2, 3 } },
            new[] { new[] { 0, 2 } },
            new[] { new[] { 0, 1 }, new [] { 2, 3 } },
            new[] { new[] { 1, 2 } },
            new[] { new[] { 1, 3 } },
            new[] { new[] { 0, 1 } },
            new[] { new[] { 0, 3 } },
            new[] { new int[0] },
        };

    #endregion

    #region Private methods

    // (x,y) coordinates.
    private PointD[] GetPoints( double dx, double dy, double xmin, double ymin, int n, int m )
    {
        double[,] c = { { 0, 1f }, { 1f, 1f }, { 1f, 0 }, { 0, 0 }, { .5f, .5f } };

        var p = new PointD( dx * n + xmin, dy * m + ymin );

        return Enumerable.Range( 0, 5 ).Select( k => new PointD( p.X + c[ k, 0 ] * dx, p.Y + c[ k, 1 ] * dy ) ).ToArray();
    }


    private double[] GetValues( int n, int m, double[,] zvalues )
    {
        var vals = new double[5];

        vals[0] = zvalues[ n + 0, m + 1 ];
        vals[1] = zvalues[ n + 1, m + 1 ];
        vals[2] = zvalues[ n + 1, m + 0 ];
        vals[3] = zvalues[ n + 0, m + 0 ];

        // Значение в центре квадрата.
        vals[4] = ( vals[0] + vals[1] + vals[2] + vals[3] ) / 4;

        return vals;
    }


    private int GetIndex( double[] vals, double isolevel )
    {
        var indx = 0;

        indx |= vals[0] < isolevel ? 1 << 0 : 0;
        indx |= vals[1] < isolevel ? 1 << 1 : 0;
        indx |= vals[2] < isolevel ? 1 << 2 : 0;
        indx |= vals[3] < isolevel ? 1 << 3 : 0;

        if ( indx == 10 )
        {
            if ( vals[4] < isolevel ) indx = 5;
        }
        else if ( indx == 5 )
        {
            if ( vals[4] < isolevel ) indx = 10;
        }

        return indx;
    }


    private PointD VertexInterp( double level, PointD p1, PointD p2, double v1, double v2 )
    {
        var s = ( level - v1 ) / ( v2 - v1 );

        return new PointD( p1.X + s * ( p2.X - p1.X ), p1.Y + s * ( p2.Y - p1.Y ) );
    }


    private PointD[] GetVertList( double level, PointD[] pp, double[] vals )
    {
        var vlist = new[]
        {
                vals[0] > vals[1] ? VertexInterp( level, pp[0], pp[1], vals[0], vals[1] ) : VertexInterp( level, pp[1], pp[0], vals[1], vals[0] ),
                vals[1] > vals[2] ? VertexInterp( level, pp[1], pp[2], vals[1], vals[2] ) : VertexInterp( level, pp[2], pp[1], vals[2], vals[1] ),
                vals[2] > vals[3] ? VertexInterp( level, pp[2], pp[3], vals[2], vals[3] ) : VertexInterp( level, pp[3], pp[2], vals[3], vals[2] ),
                vals[3] > vals[0] ? VertexInterp( level, pp[3], pp[0], vals[3], vals[0] ) : VertexInterp( level, pp[0], pp[3], vals[0], vals[3] )
            };

        return vlist;
    }


    private List<PointD[]> ImplicitPlot2d( double dx, double dy, double xmin, double ymin, int nx, int ny, double[,] zvalues, double isolevel = 0 )
    {
        var pairs = new List<PointD[]>();

        for ( var n = 0; n < nx; n++ )
        {
            for ( var m = 0; m < ny; m++ )
            {
                try
                {
                    // Значение z в вершинах квадрата.
                    var vals = GetValues( n, m, zvalues );

                    if ( vals.Any( z => double.IsNaN(z) ) ) continue;

                    // Классифицируем тип пересечения.
                    var indx = GetIndex( vals, isolevel );

                    // Пропускаем, если нет пересечения.
                    if ( _edgeTable[ indx ] == 0 ) continue;

                    // Текущий квадрат.
                    var xy = GetPoints( dx, dy, xmin, ymin, n, m );

                    // Получаем список точек для найденного квадрата.
                    var vlist = GetVertList( isolevel, xy, vals );

                    // Заполняем список точек кривой отрезками на основе 
                    // найденной конфигурации пересечения.
                    pairs.AddRange( _table[ indx ].Select( p => new[] { vlist[ p[0] ], vlist[ p[1] ] } ) );
                }
                catch { continue; }
            }
        }

        var curves = new List<List<PointD>>();

        while ( pairs.Any() )
        {
            var beg = pairs[0][0];
            var end = pairs[0][1];

            var curve = new List<PointD> { beg, end };

            curves.Add( curve );

            pairs.RemoveAt( 0 );

            var index = pairs.FindIndex( x => x.Contains( beg ) || x.Contains( end ) );

            while ( index != -1 )
            {
                var pair = pairs[ index ];

                if ( pair.Contains( beg ) )
                {
                    beg = pair[0].Equals( beg ) ? pair[1] : pair[0];

                    curve.Insert( 0, beg );
                }
                else
                {
                    end = pair[0].Equals( end ) ? pair[1] : pair[0];

                    curve.Add( end );
                }

                pairs.RemoveAt( index );

                index = pairs.FindIndex( x => x.Contains( beg ) || x.Contains( end ) );
            }

            pairs = pairs.Where( p => p.Length > 0 ).ToList();
        }

        return curves.Select( x => x.ToArray() ).ToList();
    }

    #endregion

    #region Public methods

    public FunctionInfo Info => new FunctionInfo( "csimplot2d", "dx,dy,xmin,ymin,zvalues", "return list of curves",
            typeof( Complex[,] ),
            new[] {
                typeof( Complex ), typeof( Complex ),
                typeof( Complex ), typeof( Complex ),
                typeof( Complex[,] ) 
            } );

    public FunctionInfo GetFunctionInfo( string lang ) => Info;

    public bool NumericEvaluation( object[] args, out object result, ref Context context )
    {
        double dx = ( ( Complex ) args[0] ).Real;
        double dy = ( ( Complex ) args[1] ).Real;

        double xmin = ( ( Complex ) args[2] ).Real;
        double ymin = ( ( Complex ) args[3] ).Real;

        Complex[,] matrix = ( Complex[,] ) args[4];

        var rows = matrix.GetLength(0);
        var cols = matrix.GetLength(1);

        double [,] zvalues = new double [ rows, cols ];

        for ( int n = 0; n < rows; n++ )
            for ( int m = 0; m < cols; m++ )
            {
                zvalues[ n, m ] = matrix[ n, m ].Real;
            }

        var curves = ImplicitPlot2d( dx, dy, xmin, ymin, rows - 1, cols - 1, zvalues );

        var list = new List<Complex[]>();

        foreach ( var curve in curves ) 
        { 
            list.AddRange( curve.Select( p => new[] { new Complex( p.X, 0 ), new Complex( p.Y, 0 ) } ) );

            list.Add( new[] { new Complex( 0, 1 ), new Complex( 0, 1 ) } );
        }

        list.Remove( list.Last() );

        var data = new Complex[ list.Count, 2 ];

        for ( var n = 0; n < list.Count; n++ )
        { 
            data[ n, 0 ] = list[n][0];
            data[ n, 1 ] = list[n][1];
        }

        result = data;

        return true;
    }

    #endregion

}