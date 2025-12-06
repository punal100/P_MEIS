# Base Folder

This folder contains the core implementation code for the Modular Enhanced Input System (P_MEIS) plugin.

## Structure

Place your actual code files here. This folder is included in both Public and Private include paths, 
so you can organize your headers and implementations as needed.

### Recommended Organization:
- .h header files - Class declarations and interfaces
- .cpp implementation files - Class implementations

### Include Usage:
Files in this folder can be included from anywhere in the module:
`cpp
#include "YourFile.h"
`

## Guidelines

1. Keep related functionality together
2. Use forward declarations when possible to reduce compilation dependencies
3. Follow UE5 naming conventions (F prefix for structs, U prefix for UObject-derived classes, etc.)
