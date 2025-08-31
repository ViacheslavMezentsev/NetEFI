#include "stdafx.h"
#include "Manager.h"
#include "ContextImpl.h"

using namespace NetEFI;

extern CMathcadEfi MathcadEfi;

bool ContextImpl::IsUserInterrupted::get()
{
    return ( MathcadEfi.isUserInterrupted != nullptr ) && MathcadEfi.isUserInterrupted();
}


bool ContextImpl::IsDefined( String ^ name )
{
    return this[ name ] != nullptr;
}


generic<typename TResult>
TResult ContextImpl::Invoke( String^ functionName, ... array<Object^>^ args )
{
    // 1. Находим функцию по имени
    CustomFunctionBase^ func = this[ functionName ];

    if ( func == nullptr )
    {
        throw gcnew Exception( String::Format( "Function '{0}' is not defined.", functionName ) );
    }

    // 2. Вызываем ее "скрытый" NumericEvaluation метод
    Object^ result;

    if ( func->NumericEvaluation( args, result, this ) )
    {
        // 3. Проверяем, не вернулась ли ошибка
        auto errorResult = dynamic_cast< ErrorResult^ >( result );

        if ( errorResult != nullptr )
        {
            // Если одна функция вызывает другую, и та возвращает ошибку,
            // мы "пробрасываем" эту ошибку дальше.
            throw gcnew EFIException( errorResult->ErrorKey, errorResult->ArgumentIndex );
        }

        // 4. Приводим результат к ожидаемому типу и возвращаем
        return safe_cast< TResult >( result );
    }

    // Если NumericEvaluation вернул false, это внутренняя ошибка.
    throw gcnew Exception( String::Format( "Internal error while invoking function '{0}'.", functionName ) );
}


void ContextImpl::LogInfo( String ^ text )
{ 
    Manager::LogInfo( text );
}


void ContextImpl::LogInfo( String ^ format, ... array<Object ^> ^ list )
{ 
    Manager::LogInfo( format, list );
}


void ContextImpl::LogError( String ^ text )
{ 
    Manager::LogError( text );
}


void ContextImpl::LogError( String ^ format, ... array<Object ^> ^ list )
{ 
    Manager::LogError( format, list );
}
