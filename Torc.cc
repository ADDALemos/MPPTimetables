#include <stddef.h>  // defines NULL
#include <cstdlib>

#include "MaxSAT.h"
#include "Torc.h"
 
// Global static pointer used to ensure a single instance of the class.
Torc* Torc::m_pInstance = NULL; 
Torc* Torc::Instance()
{
   if (!m_pInstance)   // Only allow one instance of class to be generated.
      m_pInstance = new Torc;
   return m_pInstance;
}

int Torc::GetRandBump() const 
{ 
	return varTargetsBumpMaxRandVal == 0 ? 0 : rand() % varTargetsBumpMaxRandVal; 
}

