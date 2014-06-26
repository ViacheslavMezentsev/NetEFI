﻿using System;
using System.Collections.Generic;
using System.Reflection;
using NetEFI;


namespace Functions
{

    public class cstest2 : IFunction
    {

        private FunctionInfo _info;

        public FunctionInfo Info
        {

            get { return _info; }
        }

        public cstest2()
        {

            _info = new FunctionInfo(

                "cstest2", "separ, v", "return string: v[0] separ v[1] separ ...",
                new Uri(Assembly.GetExecutingAssembly().CodeBase).LocalPath,
                typeof(String),
                new[] { typeof(String), typeof(TComplex[,]) }
                );
        }

        public FunctionInfo GetFunctionInfo(string lang)
        {

            return Info;
        }

        public bool NumericEvaluation(object[] args, out object result)
        {

            var d = (String ) args[0];
            var v = ( TComplex[,] ) args[1];

            int len = v.GetLength(0);

            var list = new List<string>();

            for (int n = 0; n < len; n++) list.Add(String.Format("{0} + {1} * i", v[n, 0].Real, v[n, 0].Imaginary));

            result = String.Join(d, list.ToArray());

            return true;
        }

    }

}
