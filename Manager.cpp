#include "stdafx.h"
#include "Manager.h"

void Manager::Log( String ^ format, ... array<Object ^> ^ list )
{
    String ^ text = String::Format( format, list );

    text = String::Format( "{0:dd.MM.yyyy HH:mm:ss} {1}{2}", DateTime::Now, text, Environment::NewLine );

    try
    {
        File::AppendAllText( LogFile, text, Encoding::UTF8 );
    }
    catch ( ... )
    {
    }
}


void Manager::LogInfo( String ^ format, ... array<Object ^> ^ list )
{
    Log( "[INFO ] " + format, list );
}


void Manager::LogError( String ^ format, ... array<Object ^> ^ list )
{
    Log( "[ERROR] " + format, list );
}


void Manager::LogInfo( std::string text )
{
    LogInfo( marshal_as<String ^>( text ) );
}


void Manager::LogError( std::string text )
{
    LogError( marshal_as<String ^>( text ) );
}
