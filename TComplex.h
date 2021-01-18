#pragma once

public ref class TComplex
{
private:
	double _imaginary;
	double _real;

public:
	property double Imaginary
	{
		double get()
		{
			return _imaginary;
		}
	}

	property double Real
	{ 
		double get()
		{ 
			return _real;
		}
	}

public:
	TComplex( double real, double imaginary )
	{
		_real = real;
		_imaginary = imaginary;
	}
};
