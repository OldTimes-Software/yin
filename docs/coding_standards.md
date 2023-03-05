# Yin Coding Standards

Yin is primarily in C, with some external components in C++. 
These rules apply to everything except for any third-party
libraries, which will of course have their own code style.

## Function Name

All public functions should be prefixed with `OS`, for OldTimes Software, and then their respective library name, for instance, `OS_Game_`

Whenever public, these should be based upon the name of the project, folder and file. 
For example, we have `flare.c` under a `renderer` directory of the engine...

> OS_Engine_Renderer_CreateFlare

Or alternatively, we have `sgui.c` under the `client` directory.

> OS_Engine_SGUI_

In such a case that you have `client.c` under the `client` directory
(i.e., the file name is the same as the folder name), then it's sufficient to do the following.

> OS_Engine_Client_*

If this subdirectory rule isn't available to you, ideally you should use
the name of the library the function is attached to. 
For example, a common library function in `common_pkg.c` could be the following.

```c
void OS_Common_Pkg_Load( const char *path ) { [...] }
```

### Further Examples

**Engine**
```c
void OS_Engine_Example_DoThing( void );
```

**Game**
```c
void OS_Game_Example_DoThing( void );
```

**GUI**
```c
void OS_GUI_Example_DoThing( void );
```

## Variable Names

```c
unsigned int someVariableName = 0;
float someOtherVariableName = 1.0f;
```

## Structs

```c
/*always typedef*/
typedef struct MyStruct
{
	float x;
	float y;
	float z;
	int someVariable;
} MyStruct;
```

When deciding on a name for the struct, try to scope it based on
the file it's in. For example, `renderer/particle.c` would resolve
to `OSGE_ParticleEmitter`/`ParticleMyThing`.

## Example

```c
// Both single line comments
/* and multi line comments */
// are accepted

static int someVar = 0;
int globalSomeVar = 0;  // Use globals sparingly
int rendererVar = 0;    // or name after component...

// If for example, under filesystem.c, function name prefix matches filename
void FileSystem_DoAThing( int32_t myVar, uint32_t anotherVar )
{
    printf( "Hello World!\n" );
    if ( myVar >= 1 )
        printf( "GREQ\n" );
    else
        printf( "LT\n" );
	
    if ( anotherVar >= 1 )
    {
        MyFunctionCall();
        AnotherFunctionCall();
    }
}

// We're using C11, so stdbool is assumed to be available
#include <stdbool.h>
static bool myBoolean;

/**
 * Structs can be declared like so.
 * The key defining difference in naming
 * between a function and struct is the lack
 * of an underscore between the identifier
 * and the rest of the name.
 */
typedef struct FileSystemSomeStruct
{
	int someVar;
	unsigned int someOtherVar;
} FileSystemSomeStruct;
// If the struct is used for I/O operations, please use stdint sized types!
```
