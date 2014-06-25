#include "stdafx.h"

#ifdef __cplusplus
    #define PR_BEGIN_EXTERN_C       extern "C" {
    #define PR_END_EXTERN_C         }
#else
    #define PR_BEGIN_EXTERN_C
    #define PR_END_EXTERN_C
#endif


PR_BEGIN_EXTERN_C
                           
/*************************************
*  Structure and function definitions
*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**************************************/


// types to be used in declaration of the function's
// arguments and of the return value
#define COMPLEX_SCALAR  1
#define COMPLEX_ARRAY   2
#define STRING			8

// File name variables. These are passed as const char *pointers
// if the string doesn't look like it has a path in it then 
// the current working directory will be prepended to the string 
// before it is passed to the user function.

// your function will be passed a const char * pointer
#define INFILE			13
// an OUTFILE is like an INFILE except it allows you
// to put your function on the left side of a := like
// the WRITEPRN() builtin
#define OUTFILE			14

// use this structure to create a function
#define MAX_ARGS        10


// The COMPLEXSCALAR structure is used to pass scalar data between Mathcad and a user
// DLL. The real part of a scalar is stored in the real member of a COMPLEXSCALAR, and
// the imaginary in the imag member.
typedef struct tagCOMPLEXSCALAR {
    
    double real; // Contains the real part of a scalar.
    double imag; // Contains the imaginary part of a scalar.

} COMPLEXSCALAR;
   
// this is the complex scalar type received from mathcad
typedef const COMPLEXSCALAR * const LPCCOMPLEXSCALAR; 

// this is the complex scalar type that should be returned to mathcad
typedef COMPLEXSCALAR * const LPCOMPLEXSCALAR;
    

// The COMPLEXARRAY structure is used to pass array data between Mathcad and a user
// DLL. It contains the information about the size of the array and whether any of the
// elements in the array has an imaginary or a real component.
typedef struct tagCOMPLEXARRAY  {
   
    unsigned int rows; // Number of rows in the array.
    unsigned int cols; // Number of columns in the array.
    
    // Points to the real part of a complex array hReal[i][j] contains the
    // element in the ith column and the jth row of the array. hReal is equal to
    // NULL if the array has no real component.
    double **hReal; // hReal[cols][rows],  == NULL when the real part is zero 

    // Points to the imaginary part of a complex array hImag[i][j], contains
    // the element in the ith column and the jth row of the array. hImag equals
    // NULL if the array has no imaginary component.
    double **hImag; // hImag[cols][rows],  == NULL when the imaginary part is zero

} COMPLEXARRAY;
    
// this is the complex array type received from mathcad
typedef const COMPLEXARRAY * const LPCCOMPLEXARRAY;    

// this is the complex array type that should be returned to mathcad
typedef COMPLEXARRAY * const LPCOMPLEXARRAY;


typedef struct tagMCSTRING {
	char *str;
}MCSTRING;

typedef const MCSTRING * const LPCMCSTRING;
typedef MCSTRING * const LPMCSTRING;


typedef LRESULT (* LPCFUNCTION ) ( void * const, const void * const, ... );    

// The FUNCTIONINFO structure contains the information that Mathcad uses to register a
// user function. Refer below for each member and its description.
typedef struct tagFUNCTIONINFO {

    // Points to a NULL-terminated string that specifies the name of the user
    // function.
    char *  lpstrName;

    // Points to a NULL-terminated string that specifies the parameters of the
    // user function.
    char *  lpstrParameters; 

    // Points to a NULL-terminated string that specifies the function description.
    char *  lpstrDescription;

    // Pointer to the code that executes the user function.
    LPCFUNCTION lpfnMyCFunction;

    // Specifies the type of value returned by the function. The values are
    // COMPLEX_ARRAY or COMPLEX_SCALAR.
    long unsigned int returnType;

    // Specifies the number of arguments expected by the function. Must be
    // between 1 and MAX_ARGS.
    unsigned int nArgs;

    // Specifies an array of long unsigned integers containing input parameter
    // types.
    long unsigned int argType[ MAX_ARGS ];

} FUNCTIONINFO;


// CreateUserFunction is called when the DLL is attaching to the address space of
// the current process in order to register the user function with Mathcad.
//const void * CreateUserFunction( HINSTANCE, FUNCTIONINFO * );
typedef void * ( * PCREATE_USER_FUNCTION ) ( HINSTANCE, FUNCTIONINFO * );   

// CreateUserErrorMessageTable is called when the DLL is attaching to the address
// space of the current process in order to register the user error message table with
// Mathcad.
// Parameters:
// hDLL - Handle of the DLL supplied by the DLL entry point routine.
// nErrorMessages - Number of error messages in the table.
// ErrorMessageTable - An array of n strings with the text of the error messages.
/*
BOOL CreateUserErrorMessageTable( HINSTANCE hDLL, 
    unsigned int nErrorMessages, char  * ErrorMessageTable[] );
*/
typedef BOOL ( * PCREATE_USER_ERROR_MESSAGE_TABLE ) ( HINSTANCE, unsigned int, char  ** );

// Should be used to allocate memory inside the MyCFunction. Allocates a memory block
// of a given size (in bytes) of memory.
//char * MathcadAllocate( unsigned int size );
typedef char * ( * PMATHCAD_ALLOCATE ) ( unsigned int );

// Should be used to free memory allocated with MathcadAllocate. The argument
// address points to the memory previously allocated with MathcadAllocate. A NULL
// pointer argument is ignored.
//void MathcadFree( char * address );
typedef void ( * PMATHCAD_FREE ) ( char * );


// Allocates memory for a COMPLEXARRAY of cols columns and rows rows. Sets the hReal,
// hImag, rows and cols members of the argument array.
/*
BOOL MathcadArrayAllocate(   COMPLEXARRAY * const, 
                                unsigned int rows,  
                                unsigned int cols,
                                BOOL allocateReal, 
                                BOOL allocateImag );
*/
typedef BOOL ( * PMATHCAD_ARRAY_ALLOCATE ) ( COMPLEXARRAY *, 
                                unsigned int,  
                                unsigned int, 
                                BOOL, 
                                BOOL ); 

// Frees memory that was allocated by the MathcadArrayAllocate function to the
// hReal and hImag members of the argument array.
//void MathcadArrayFree( COMPLEXARRAY * const );
typedef void ( * PMATHCAD_ARRAY_FREE ) ( COMPLEXARRAY * const );

// The isUserInterrupted function is used to check whether a user has pressed the
// [Esc] key. Include this function if you want to be able to interrupt your function like
// other Mathcad functions.    
//BOOL isUserInterrupted( void );
typedef BOOL ( * PIS_USER_INTERRUPTED ) ( void );

PR_END_EXTERN_C